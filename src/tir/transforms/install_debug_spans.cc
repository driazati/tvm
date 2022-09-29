/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/*!
 * \file install_debug_spans.cc
 * \brief Prints TIR code in memory and replaces all spans in the module with
    the location to which the ops would be printed
 */

#include "install_debug_spans.h"

#include <tvm/ir/transform.h>
#include <tvm/runtime/container/array.h>
#include <tvm/runtime/container/string.h>
#include <tvm/tir/analysis.h>
#include <tvm/tir/expr.h>
#include <tvm/tir/expr_functor.h>
#include <tvm/tir/function.h>
#include <tvm/tir/stmt.h>
#include <tvm/tir/stmt_functor.h>
#include <tvm/tir/transform.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../../printer/text_printer.h"

namespace tvm {
namespace tir {

Stmt DebugInfoInstaller::InstallInfo(const Stmt& stmt) {
  DebugInfoInstaller installer(stmt);
  return installer.VisitStmt(stmt);
}

DebugInfoInstaller::DebugInfoInstaller(const Stmt& stmt) {
  std::cout << "Running DebugInfoInstaller\n";
  TextMetaDataContext meta;
  tvm::tir::TIRTextPrinter printer(false, &meta);
  auto result = printer.Print(stmt).str();
  // TODO: Make this <name of primfunc>.tir
  std::ofstream out("main.tir");
  out << result;
  out.close();
  std::cout << result << "\n";
  lines_ = printer.GetStmtNodeLines();
  expr_lines_ = printer.GetExprLines();

  for (const auto& line : lines_) {
    // VLOG(0) << "Recorded " << std::get<0>(line) << " @ line " << std::get<1>(line) << "\n";
    stmt_lines_[std::get<0>(line)] = std::get<1>(line);
  }
  for (const auto& line : expr_lines_) {
    // VLOG(0) << "Recorded " << std::get<0>(line) << " @ line " << std::get<1>(line) << "\n";
    expr_lines_map_[std::get<0>(line)] = std::get<1>(line);
  }
}

PrimExpr DebugInfoInstaller::VisitExpr(const PrimExpr& expr) {
  PrimExpr result = expr;
  result = StmtExprMutator::VisitExpr(result);
  return result;
}

Stmt DebugInfoInstaller::VisitStmt(const Stmt& stmt) {
  Stmt result = stmt;
  result = StmtExprMutator::VisitStmt(result);
  return result;
}

Span DebugInfoInstaller::MaybeSpan(const StmtNode* op) {
  auto entry = stmt_lines_.find(op);
  if (entry == stmt_lines_.end()) {
    return Span(SourceName::Get("missing-file"), 999, 999, 999, 999);
  } else {
    size_t column = 0;
    return Span(SourceName::Get("main.tir"), entry->second, entry->second, column, column);
  }
}

Span DebugInfoInstaller::MaybeSpan(const PrimExprNode* op) {
  auto entry = expr_lines_map_.find(op);
  if (entry == expr_lines_map_.end()) {
    return Span(SourceName::Get("missing-expr-file"), 998, 998, 998, 998);
  } else {
    size_t column = 0;
    return Span(SourceName::Get("main.tir"), entry->second, entry->second, column, column);
  }
}

// PrimExpr DebugInfoInstaller::VisitExpr_(const VarNode* op) {
//   auto new_expr = StmtExprMutator::VisitExpr_(op);
//   auto newop = new_expr.as<VarNode>();
//   return Var(newop->name_hint, newop->type_annotation, MaybeSpan(op));
// }

// PrimExpr DebugInfoInstaller::VisitExpr_(const SizeVarNode* op) {
//   auto new_expr = StmtExprMutator::VisitExpr_(op);
//   auto newop = new_expr.as<SizeVarNode>();
//   return SizeVar(newop->name_hint, newop->dtype, MaybeSpan(op));
// }

// PrimExpr DebugInfoInstaller::VisitExpr_(const LoadNode* op) {
//   auto new_expr = StmtExprMutator::VisitExpr_(op);
//   auto newop = new_expr.as<LoadNode>();
//   return Load(newop->dtype, newop->buffer_var, newop->index, newop->predicate, MaybeSpan(op));
// }

// PrimExpr DebugInfoInstaller::VisitExpr_(const BufferLoadNode* op) {
//   auto new_expr = StmtExprMutator::VisitExpr_(op);
//   auto newop = new_expr.as<BufferLoadNode>();
//   return BufferLoad(newop->buffer, newop->indices, MaybeSpan(op));
// }

// PrimExpr DebugInfoInstaller::VisitExpr_(const ProducerLoadNode* op) {
//   auto new_expr = StmtExprMutator::VisitExpr_(op);
//   auto newop = new_expr.as<ProducerLoadNode>();
//   return ProducerLoad(newop->producer, newop->indices, MaybeSpan(op));
// }

// PrimExpr DebugInfoInstaller::VisitExpr_(const LetNode* op) {
//   auto new_expr = StmtExprMutator::VisitExpr_(op);
//   auto newop = new_expr.as<LetNode>();
//   return Let(newop->var, newop->value, newop->body, MaybeSpan(op));
// }

PrimExpr DebugInfoInstaller::VisitExpr_(const CallNode* op) {
  auto new_expr = StmtExprMutator::VisitExpr_(op);
  auto newop = new_expr.as<CallNode>();
  return Call(newop->dtype, newop->op, newop->args, MaybeSpan(op));
}

PrimExpr DebugInfoInstaller::VisitExpr_(const AddNode* op) {
  auto new_expr = StmtExprMutator::VisitExpr_(op);
  auto newop = new_expr.as<AddNode>();
  return Add(newop->a, newop->b, MaybeSpan(op));
}

// template <typename NodeType, typename Type>
// PrimExpr VisitExpr_(const NodeType* op) {
//   auto new_expr = StmtExprMutator::VisitExpr_(op);
//   auto new_op = new_expr.as<NodeType>();
//   new_op
//   return Type::from(new_op)->withSpan(MaybeSpan(op))->build();
// }

PrimExpr DebugInfoInstaller::VisitExpr_(const SubNode* op) {
  auto new_expr = StmtExprMutator::VisitExpr_(op);
  auto newop = new_expr.as<SubNode>();
  return Sub(newop->a, newop->b, MaybeSpan(op));
}

PrimExpr DebugInfoInstaller::VisitExpr_(const MulNode* op) {
  auto new_expr = StmtExprMutator::VisitExpr_(op);
  auto newop = new_expr.as<MulNode>();
  return Mul(newop->a, newop->b, MaybeSpan(op));
}

PrimExpr DebugInfoInstaller::VisitExpr_(const DivNode* op) {
  auto new_expr = StmtExprMutator::VisitExpr_(op);
  auto newop = new_expr.as<DivNode>();
  return Div(newop->a, newop->b, MaybeSpan(op));
}

PrimExpr DebugInfoInstaller::VisitExpr_(const ModNode* op) {
  auto new_expr = StmtExprMutator::VisitExpr_(op);
  auto newop = new_expr.as<ModNode>();
  return Mod(newop->a, newop->b, MaybeSpan(op));
}

PrimExpr DebugInfoInstaller::VisitExpr_(const FloorDivNode* op) {
  auto new_expr = StmtExprMutator::VisitExpr_(op);
  auto newop = new_expr.as<FloorDivNode>();
  return FloorDiv(newop->a, newop->b, MaybeSpan(op));
}

PrimExpr DebugInfoInstaller::VisitExpr_(const FloorModNode* op) {
  auto new_expr = StmtExprMutator::VisitExpr_(op);
  auto newop = new_expr.as<FloorModNode>();
  return FloorMod(newop->a, newop->b, MaybeSpan(op));
}

PrimExpr DebugInfoInstaller::VisitExpr_(const MinNode* op) {
  auto new_expr = StmtExprMutator::VisitExpr_(op);
  auto newop = new_expr.as<MinNode>();
  return Min(newop->a, newop->b, MaybeSpan(op));
}

PrimExpr DebugInfoInstaller::VisitExpr_(const MaxNode* op) {
  auto new_expr = StmtExprMutator::VisitExpr_(op);
  auto newop = new_expr.as<MaxNode>();
  return Max(newop->a, newop->b, MaybeSpan(op));
}

PrimExpr DebugInfoInstaller::VisitExpr_(const EQNode* op) {
  auto new_expr = StmtExprMutator::VisitExpr_(op);
  auto newop = new_expr.as<EQNode>();
  return EQ(newop->a, newop->b, MaybeSpan(op));
}

PrimExpr DebugInfoInstaller::VisitExpr_(const NENode* op) {
  auto new_expr = StmtExprMutator::VisitExpr_(op);
  auto newop = new_expr.as<NENode>();
  return NE(newop->a, newop->b, MaybeSpan(op));
}

PrimExpr DebugInfoInstaller::VisitExpr_(const LTNode* op) {
  auto new_expr = StmtExprMutator::VisitExpr_(op);
  auto newop = new_expr.as<LTNode>();
  return LT(newop->a, newop->b, MaybeSpan(op));
}

PrimExpr DebugInfoInstaller::VisitExpr_(const LENode* op) {
  auto new_expr = StmtExprMutator::VisitExpr_(op);
  auto newop = new_expr.as<LENode>();
  return LE(newop->a, newop->b, MaybeSpan(op));
}

PrimExpr DebugInfoInstaller::VisitExpr_(const GTNode* op) {
  auto new_expr = StmtExprMutator::VisitExpr_(op);
  auto newop = new_expr.as<GTNode>();
  return GT(newop->a, newop->b, MaybeSpan(op));
}

PrimExpr DebugInfoInstaller::VisitExpr_(const GENode* op) {
  auto new_expr = StmtExprMutator::VisitExpr_(op);
  auto newop = new_expr.as<GENode>();
  return GE(newop->a, newop->b, MaybeSpan(op));
}

PrimExpr DebugInfoInstaller::VisitExpr_(const AndNode* op) {
  auto new_expr = StmtExprMutator::VisitExpr_(op);
  auto newop = new_expr.as<AndNode>();
  return And(newop->a, newop->b, MaybeSpan(op));
}

PrimExpr DebugInfoInstaller::VisitExpr_(const OrNode* op) {
  auto new_expr = StmtExprMutator::VisitExpr_(op);
  auto newop = new_expr.as<OrNode>();
  return Or(newop->a, newop->b, MaybeSpan(op));
}

PrimExpr DebugInfoInstaller::VisitExpr_(const ReduceNode* op) {
  auto new_expr = StmtExprMutator::VisitExpr_(op);
  auto newop = new_expr.as<ReduceNode>();
  return Reduce(newop->combiner, newop->source, newop->axis, newop->condition, newop->value_index,
                newop->init, MaybeSpan(op));
}

PrimExpr DebugInfoInstaller::VisitExpr_(const CastNode* op) {
  auto new_expr = StmtExprMutator::VisitExpr_(op);
  auto newop = new_expr.as<CastNode>();
  return Cast(newop->dtype, newop->value, MaybeSpan(op));
}

PrimExpr DebugInfoInstaller::VisitExpr_(const NotNode* op) {
  auto new_expr = StmtExprMutator::VisitExpr_(op);
  auto newop = new_expr.as<NotNode>();
  return Not(newop->a, MaybeSpan(op));
}

PrimExpr DebugInfoInstaller::VisitExpr_(const SelectNode* op) {
  auto new_expr = StmtExprMutator::VisitExpr_(op);
  auto newop = new_expr.as<SelectNode>();
  return Select(newop->condition, newop->true_value, newop->false_value, MaybeSpan(op));
}

PrimExpr DebugInfoInstaller::VisitExpr_(const RampNode* op) {
  auto new_expr = StmtExprMutator::VisitExpr_(op);
  auto newop = new_expr.as<RampNode>();
  return Ramp(newop->base, newop->stride, newop->lanes, MaybeSpan(op));
}

PrimExpr DebugInfoInstaller::VisitExpr_(const BroadcastNode* op) {
  auto new_expr = StmtExprMutator::VisitExpr_(op);
  auto newop = new_expr.as<BroadcastNode>();
  return Broadcast(newop->value, newop->lanes, MaybeSpan(op));
}

PrimExpr DebugInfoInstaller::VisitExpr_(const ShuffleNode* op) {
  auto new_expr = StmtExprMutator::VisitExpr_(op);
  auto newop = new_expr.as<ShuffleNode>();
  return Shuffle(newop->vectors, newop->indices, MaybeSpan(op));
}

PrimExpr DebugInfoInstaller::VisitExpr_(const IntImmNode* op) {
  auto new_expr = StmtExprMutator::VisitExpr_(op);
  auto newop = new_expr.as<IntImmNode>();
  return IntImm(newop->dtype, newop->value, MaybeSpan(op));
}

PrimExpr DebugInfoInstaller::VisitExpr_(const FloatImmNode* op) {
  auto new_expr = StmtExprMutator::VisitExpr_(op);
  auto newop = new_expr.as<FloatImmNode>();
  return FloatImm(newop->dtype, newop->value, MaybeSpan(op));
}

PrimExpr DebugInfoInstaller::VisitExpr_(const StringImmNode* op) {
  auto new_expr = StmtExprMutator::VisitExpr_(op);
  auto newop = new_expr.as<StringImmNode>();
  return StringImm(newop->value, MaybeSpan(op));
}

PrimExpr DebugInfoInstaller::VisitExpr_(const AnyNode* op) {
  auto new_expr = StmtExprMutator::VisitExpr_(op);
  auto newop = new_expr.as<AnyNode>();
  return Any(MaybeSpan(op));
}

// Stmt DebugInfoInstaller::VisitStmt_(const AttrStmtNode* op) {}
// Stmt DebugInfoInstaller::VisitStmt_(const IfThenElseNode* op) {}
// Stmt DebugInfoInstaller::VisitStmt_(const LetStmtNode* op) {}
// Stmt DebugInfoInstaller::VisitStmt_(const AssertStmtNode* op) {}
Stmt DebugInfoInstaller::VisitStmt_(const ForNode* op) {
  auto new_stmt = StmtExprMutator::VisitStmt_(op);
  auto newop = new_stmt.as<ForNode>();
  return For(newop->loop_var, newop->min, newop->extent, newop->kind, newop->body,
             newop->thread_binding, newop->annotations, MaybeSpan(op));
}
Stmt DebugInfoInstaller::VisitStmt_(const WhileNode* op) {
  auto new_stmt = StmtExprMutator::VisitStmt_(op);
  auto newop = new_stmt.as<WhileNode>();
  return While(newop->condition, newop->body, MaybeSpan(op));
}
Stmt DebugInfoInstaller::VisitStmt_(const AllocateNode* op) {
  auto new_stmt = StmtExprMutator::VisitStmt_(op);
  auto newop = new_stmt.as<AllocateNode>();
  return Allocate(newop->buffer_var, newop->dtype, newop->extents, newop->condition, newop->body,
                  newop->annotations, MaybeSpan(op));
}
Stmt DebugInfoInstaller::VisitStmt_(const AllocateConstNode* op) {
  auto new_stmt = StmtExprMutator::VisitStmt_(op);
  auto newop = new_stmt.as<AllocateConstNode>();
  // todo: data vs data_or_idx
  return AllocateConst(newop->buffer_var, newop->dtype, newop->extents, newop->data, newop->body,
                       newop->annotations, MaybeSpan(op));
}
Stmt DebugInfoInstaller::VisitStmt_(const DeclBufferNode* op) {
  auto new_stmt = StmtExprMutator::VisitStmt_(op);
  auto newop = new_stmt.as<DeclBufferNode>();
  return DeclBuffer(newop->buffer, newop->body, MaybeSpan(op));
}
Stmt DebugInfoInstaller::VisitStmt_(const StoreNode* op) {
  auto new_stmt = StmtExprMutator::VisitStmt_(op);
  auto newop = new_stmt.as<StoreNode>();
  return Store(newop->buffer_var, newop->value, newop->index, newop->predicate, MaybeSpan(op));
}
Stmt DebugInfoInstaller::VisitStmt_(const BufferStoreNode* op) {
  auto new_stmt = StmtExprMutator::VisitStmt_(op);
  auto newop = new_stmt.as<BufferStoreNode>();
  return BufferStore(newop->buffer, newop->value, newop->indices, MaybeSpan(op));
}
Stmt DebugInfoInstaller::VisitStmt_(const BufferRealizeNode* op) {
  auto new_stmt = StmtExprMutator::VisitStmt_(op);
  auto newop = new_stmt.as<BufferRealizeNode>();
  return BufferRealize(newop->buffer, newop->bounds, newop->condition, newop->body, MaybeSpan(op));
}
Stmt DebugInfoInstaller::VisitStmt_(const ProducerStoreNode* op) {
  auto new_stmt = StmtExprMutator::VisitStmt_(op);
  auto newop = new_stmt.as<ProducerStoreNode>();
  return ProducerStore(newop->producer, newop->value, newop->indices, MaybeSpan(op));
}
Stmt DebugInfoInstaller::VisitStmt_(const ProducerRealizeNode* op) {
  auto new_stmt = StmtExprMutator::VisitStmt_(op);
  auto newop = new_stmt.as<ProducerRealizeNode>();
  return ProducerRealize(newop->producer, newop->bounds, newop->condition, newop->body,
                         newop->storage_scope, MaybeSpan(op));
}
Stmt DebugInfoInstaller::VisitStmt_(const PrefetchNode* op) {
  auto new_stmt = StmtExprMutator::VisitStmt_(op);
  auto newop = new_stmt.as<PrefetchNode>();
  return Prefetch(newop->buffer, newop->bounds, MaybeSpan(op));
}
Stmt DebugInfoInstaller::VisitStmt_(const SeqStmtNode* op) {
  auto new_stmt = StmtExprMutator::VisitStmt_(op);
  auto newop = new_stmt.as<SeqStmtNode>();
  return SeqStmt(newop->seq, MaybeSpan(op));
}
Stmt DebugInfoInstaller::VisitStmt_(const EvaluateNode* op) {
  auto new_stmt = StmtExprMutator::VisitStmt_(op);
  auto newop = new_stmt.as<EvaluateNode>();
  return Evaluate(newop->value, MaybeSpan(op));
}
Stmt DebugInfoInstaller::VisitStmt_(const BlockNode* op) {
  auto new_stmt = StmtExprMutator::VisitStmt_(op);
  auto newop = new_stmt.as<BlockNode>();
  return Block(newop->iter_vars, newop->reads, newop->writes, newop->name_hint, newop->body,
               newop->init, newop->alloc_buffers, newop->match_buffers, newop->annotations,
               MaybeSpan(op));
}
Stmt DebugInfoInstaller::VisitStmt_(const BlockRealizeNode* op) {
  auto new_stmt = StmtExprMutator::VisitStmt_(op);
  auto newop = new_stmt.as<BlockRealizeNode>();
  return BlockRealize(newop->iter_values, newop->predicate, newop->block, MaybeSpan(op));
}

Stmt DebugInfoInstaller::VisitStmt_(const LetStmtNode* op) {
  auto new_stmt = StmtExprMutator::VisitStmt_(op);
  auto newop = new_stmt.as<LetStmtNode>();
  return LetStmt(newop->var, newop->value, newop->body, MaybeSpan(op));
}

Stmt DebugInfoInstaller::VisitStmt_(const IfThenElseNode* op) {
  std::cout << "Visiting an ifthenelse\n";
  auto new_stmt = StmtExprMutator::VisitStmt_(op);
  auto newop = new_stmt.as<IfThenElseNode>();
  // return IfThenElse(newop->condition, newop->then_case, newop->else_case,
  // Span(SourceName::Get("AAAA.irmodule"), 1, 2, 3, 4));
  return IfThenElse(newop->condition, newop->then_case, newop->else_case, MaybeSpan(op));
}

Stmt DebugInfoInstaller::VisitStmt_(const AttrStmtNode* op) {
  auto new_stmt = StmtExprMutator::VisitStmt_(op);
  auto newop = new_stmt.as<AttrStmtNode>();
  return AttrStmt(newop->node, newop->attr_key, newop->value, newop->body, MaybeSpan(op));
}

Stmt DebugInfoInstaller::VisitStmt_(const AssertStmtNode* op) {
  auto new_stmt = StmtExprMutator::VisitStmt_(op);
  auto newop = new_stmt.as<AssertStmtNode>();
  return AssertStmt(newop->condition, newop->message, newop->body, MaybeSpan(op));
}

namespace transform {

Pass InstallDebugSpans() {
  auto pass_func = [](PrimFunc f, IRModule m, PassContext ctx) {
    std::cout << "running info installer\n";
    auto* n = f.CopyOnWrite();
    n->body = DebugInfoInstaller::InstallInfo(std::move(f->body));

    return f;
  };
  return CreatePrimFuncPass(pass_func, 0, "tir.InstallDebugSpans", {});
}

TVM_REGISTER_GLOBAL("tir.transform.InstallDebugSpans").set_body_typed(InstallDebugSpans);

}  // namespace transform
}  // namespace tir
}  // namespace tvm
