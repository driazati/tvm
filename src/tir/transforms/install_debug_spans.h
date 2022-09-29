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
 * \file install_debug_spans.h
 * \brief Interface of the InstallDebugSpans pass
 */

#ifndef TVM_TIR_TRANSFORMS_INSTALL_DEBUG_SPANS_H_
#define TVM_TIR_TRANSFORMS_INSTALL_DEBUG_SPANS_H_

#include <tvm/tir/expr.h>
#include <tvm/tir/expr_functor.h>
#include <tvm/tir/stmt.h>
#include <tvm/tir/stmt_functor.h>

namespace tvm {
namespace tir {

class DebugInfoInstaller : public StmtExprMutator {
 public:
  static Stmt InstallInfo(const Stmt& stmt);

  PrimExpr VisitExpr(const PrimExpr& expr) override;
  Stmt VisitStmt(const Stmt& stmt) override;

 protected:
  DebugInfoInstaller(const Stmt& stmt);

  // PrimExpr VisitExpr_(const VarNode* op) override;
  // PrimExpr VisitExpr_(const SizeVarNode* op) override;
  // PrimExpr VisitExpr_(const LoadNode* op) override;
  // PrimExpr VisitExpr_(const BufferLoadNode* op) override;
  // PrimExpr VisitExpr_(const ProducerLoadNode* op) override;
  // PrimExpr VisitExpr_(const LetNode* op) override;
  PrimExpr VisitExpr_(const CallNode* op) override;
  PrimExpr VisitExpr_(const AddNode* op) override;
  PrimExpr VisitExpr_(const SubNode* op) override;
  PrimExpr VisitExpr_(const MulNode* op) override;
  PrimExpr VisitExpr_(const DivNode* op) override;
  PrimExpr VisitExpr_(const ModNode* op) override;
  PrimExpr VisitExpr_(const FloorDivNode* op) override;
  PrimExpr VisitExpr_(const FloorModNode* op) override;
  PrimExpr VisitExpr_(const MinNode* op) override;
  PrimExpr VisitExpr_(const MaxNode* op) override;
  PrimExpr VisitExpr_(const EQNode* op) override;
  PrimExpr VisitExpr_(const NENode* op) override;
  PrimExpr VisitExpr_(const LTNode* op) override;
  PrimExpr VisitExpr_(const LENode* op) override;
  PrimExpr VisitExpr_(const GTNode* op) override;
  PrimExpr VisitExpr_(const GENode* op) override;
  PrimExpr VisitExpr_(const AndNode* op) override;
  PrimExpr VisitExpr_(const OrNode* op) override;
  PrimExpr VisitExpr_(const ReduceNode* op) override;
  PrimExpr VisitExpr_(const CastNode* op) override;
  PrimExpr VisitExpr_(const NotNode* op) override;
  PrimExpr VisitExpr_(const SelectNode* op) override;
  PrimExpr VisitExpr_(const RampNode* op) override;
  PrimExpr VisitExpr_(const BroadcastNode* op) override;
  PrimExpr VisitExpr_(const ShuffleNode* op) override;
  PrimExpr VisitExpr_(const IntImmNode* op) override;
  PrimExpr VisitExpr_(const FloatImmNode* op) override;
  PrimExpr VisitExpr_(const StringImmNode* op) override;
  PrimExpr VisitExpr_(const AnyNode* op) override;

  Stmt VisitStmt_(const AttrStmtNode* op) override;
  Stmt VisitStmt_(const IfThenElseNode* op) override;
  Stmt VisitStmt_(const LetStmtNode* op) override;
  Stmt VisitStmt_(const ForNode* op) override;
  Stmt VisitStmt_(const WhileNode* op) override;
  Stmt VisitStmt_(const AllocateNode* op) override;
  Stmt VisitStmt_(const AllocateConstNode* op) override;
  Stmt VisitStmt_(const DeclBufferNode* op) override;
  Stmt VisitStmt_(const StoreNode* op) override;
  Stmt VisitStmt_(const BufferStoreNode* op) override;
  Stmt VisitStmt_(const BufferRealizeNode* op) override;
  Stmt VisitStmt_(const AssertStmtNode* op) override;
  Stmt VisitStmt_(const ProducerStoreNode* op) override;
  Stmt VisitStmt_(const ProducerRealizeNode* op) override;
  Stmt VisitStmt_(const PrefetchNode* op) override;
  Stmt VisitStmt_(const SeqStmtNode* op) override;
  Stmt VisitStmt_(const EvaluateNode* op) override;
  Stmt VisitStmt_(const BlockNode* op) override;
  Stmt VisitStmt_(const BlockRealizeNode* op) override;

 private:
  std::vector<std::tuple<const StmtNode*, size_t>> lines_;
  std::vector<std::tuple<const PrimExprNode*, size_t>> expr_lines_;
  std::unordered_map<const StmtNode*, size_t> stmt_lines_;
  std::unordered_map<const PrimExprNode*, size_t> expr_lines_map_;

  Span MaybeSpan(const StmtNode* op);
  Span MaybeSpan(const PrimExprNode* op);
};

}  // namespace tir
}  // namespace tvm

#endif  // TVM_TIR_TRANSFORMS_INSTALL_DEBUG_SPANS_H_
