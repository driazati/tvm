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

#define X(TypeName)                                                   \
  PrimExpr DebugInfoInstaller::VisitExpr_(const TypeName##Node* op) { \
    auto new_expr = StmtExprMutator::VisitExpr_(op);                  \
    auto new_type = Downcast<TypeName>(new_expr);                     \
    auto new_node = new_type.CopyOnWrite();                           \
    new_node->span = MaybeSpan(op);                                   \
    return new_type;                                                  \
  }
TVM_TIR_TRANSFORMS_INSTALL_DEBUG_SPANS_SUPPORTED_EXPRS
#undef X

#define X(TypeName)                                               \
  Stmt DebugInfoInstaller::VisitStmt_(const TypeName##Node* op) { \
    Stmt new_stmt = StmtExprMutator::VisitStmt_(op);              \
    auto new_type = Downcast<TypeName>(new_stmt);                 \
    auto new_node = new_type.CopyOnWrite();                       \
    new_node->span = MaybeSpan(op);                               \
    return new_type;                                              \
  }
TVM_TIR_TRANSFORMS_INSTALL_DEBUG_SPANS_SUPPORTED_STMTS
#undef X

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
