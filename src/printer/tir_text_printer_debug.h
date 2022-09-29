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
 * \file text_printer.h
 * \brief Printer to print out the unified IR text format
 *        that can be parsed by a parser.
 */

#ifndef TVM_PRINTER_TIR_TEXT_PRINTER_DEBUG_H_
#define TVM_PRINTER_TIR_TEXT_PRINTER_DEBUG_H_

#include <tuple>
#include <vector>

#include "../tir/transforms/install_debug_spans_ops.h"
#include "text_printer.h"

namespace tvm {
namespace tir {

class TIRTextPrinterDebug : public TIRTextPrinter {
 public:
  TIRTextPrinterDebug() : TIRTextPrinter(false, &meta_), current_line_(1) {}

  std::vector<std::tuple<const PrimExprNode*, size_t>> GetExprsByLine() const {
    return exprs_by_line_;
  }

  std::vector<std::tuple<const StmtNode*, size_t>> GetStmtsByLine() const { return stmts_by_line_; }

 private:
  Doc NewLine() override;

#define X(TypeName) Doc VisitExpr_(const TypeName##Node* op) override;
  TVM_TIR_TRANSFORMS_INSTALL_DEBUG_SPANS_SUPPORTED_EXPRS
#undef X

#define X(TypeName) Doc VisitStmt_(const TypeName##Node* op) override;
  TVM_TIR_TRANSFORMS_INSTALL_DEBUG_SPANS_SUPPORTED_STMTS
#undef X

  TextMetaDataContext meta_;

  // Line that the printer is currently printing
  size_t current_line_;

  // Record of all stmts and exprs and their corresponding line
  std::vector<std::tuple<const StmtNode*, size_t>> stmts_by_line_;
  std::vector<std::tuple<const PrimExprNode*, size_t>> exprs_by_line_;
};

}  // namespace tir
}  // namespace tvm

#endif  // TVM_PRINTER_TIR_TEXT_PRINTER_DEBUG_H_
