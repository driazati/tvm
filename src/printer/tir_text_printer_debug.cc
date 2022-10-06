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
 * \file tir_text_printer.cc
 * \brief Printer to print out the IR text format
 *        that can be parsed by a parser.
 */

#include "tir_text_printer_debug.h"

#include "text_printer.h"

namespace tvm {
namespace tir {

Doc TIRTextPrinterDebug::NewLine() {
  Doc doc;
  std::cout << "FLUSHING LINES with " << stmt_nodes_.size() << " and "
            << per_line_expr_nodes_.size() << "\n";
  doc << " [";
  for (size_t i = 0; i < stmt_nodes_.size(); i++) {
    const auto& entry = stmt_nodes_[i];
    if (entry->span.defined()) {
      doc << entry->span->source_name->name << ":" << entry->span->line;
    } else {
      doc << "missing";
    }
    if (i != stmt_nodes_.size() - 1) {
      doc << " ";
    }

    stmt_node_lines_.push_back(std::make_tuple(entry, current_line_));
  }
  for (size_t i = 0; i < per_line_expr_nodes_.size(); i++) {
    const auto& entry = per_line_expr_nodes_[i];
    // if (entry->span.defined()) {
    //   doc << entry->span->source_name->name << ":" << entry->span->line;
    // } else {
    //   doc << "missing";
    // }
    // if (i != per_line_expr_nodes_.size() - 1) {
    //   doc << " ";
    // }

    expr_node_lines_.push_back(std::make_tuple(entry, current_line_));
  }
  // std::cout << "Flushing " << expr_node_lines_.size()
  per_line_expr_nodes_.clear();
  stmt_nodes_.clear();
  doc << "]";
  current_line_ += 1;
  doc << TIRTextPrinter::NewLine();
  return doc;
}

#define X(TypeName)                                               \
  Doc TIRTextPrinterDebug::VisitExpr_(const TypeName##Node* op) { \
    std::cout << "calling for #TypeName\n";                       \
    per_line_expr_nodes_.push_back(op);                           \
    return TIRTextPrinter::VisitExpr_(op);                        \
  };
TVM_TIR_TRANSFORMS_INSTALL_DEBUG_SPANS_SUPPORTED_EXPRS
#undef X

#define X(TypeName)                                               \
  Doc TIRTextPrinterDebug::VisitStmt_(const TypeName##Node* op) { \
    std::cout << "calling stmt for #TypeName\n";                  \
    stmt_nodes_.push_back(op);                                    \
    return TIRTextPrinter::VisitStmt_(op);                        \
  };
TVM_TIR_TRANSFORMS_INSTALL_DEBUG_SPANS_SUPPORTED_STMTS
#undef X

}  // namespace tir
}  // namespace tvm
