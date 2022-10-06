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

#include "install_debug_spans_ops.h"

namespace tvm {
namespace tir {

class DebugInfoInstaller : public StmtExprMutator {
 public:
  static Stmt InstallInfo(const Stmt& stmt);

  PrimExpr VisitExpr(const PrimExpr& expr) override;
  Stmt VisitStmt(const Stmt& stmt) override;

 protected:
  DebugInfoInstaller(const Stmt& stmt);

#define X(TypeName) PrimExpr VisitExpr_(const TypeName##Node* op) override;
  TVM_TIR_TRANSFORMS_INSTALL_DEBUG_SPANS_SUPPORTED_EXPRS
#undef X

#define X(TypeName)                                   \
  Stmt VisitStmt_(const TypeName##Node* op) override; \
  ;
  TVM_TIR_TRANSFORMS_INSTALL_DEBUG_SPANS_SUPPORTED_STMTS
#undef X

 private:
  std::vector<std::tuple<const StmtNode*, size_t>> lines_;
  std::vector<std::tuple<const PrimExprNode*, size_t>> expr_lines_;
  std::unordered_map<const StmtNode*, size_t> stmt_lines_;
  std::unordered_map<const PrimExprNode*, size_t> expr_lines_map_;

  template <typename TypeName, typename ObjectName>
  Stmt add_span(const ObjectName* op) {
    Stmt new_stmt = StmtExprMutator::VisitStmt_(op);
    auto new_type = Downcast<TypeName>(new_stmt);
    auto new_node = new_type.CopyOnWrite();
    new_node->span = MaybeSpan(op);
    return new_type;
  }

  Span MaybeSpan(const StmtNode* op);
  Span MaybeSpan(const PrimExprNode* op);
};

}  // namespace tir
}  // namespace tvm

#endif  // TVM_TIR_TRANSFORMS_INSTALL_DEBUG_SPANS_H_
