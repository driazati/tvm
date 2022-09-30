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
 * \file common_subexpr_elim.cc
 * \brief Implementation of the Common Subexpressions Elimination (CSE) pass
           which rewrites statements and expressions in order to eliminate
           redundant computations. In order to achieve that, common (sub-)
           expressions are introduced into variables with let-in bindings,
           and the places where the expression was used are replaced with
           the freshly introduced variable.
 */

#include "common_subexpr_elim.h"

#include <tvm/ir/transform.h>  // For the class Pass and the class PassContext
#include <tvm/runtime/container/array.h>
#include <tvm/runtime/container/string.h>
#include <tvm/tir/analysis.h>  // For the analysis which gives the size of an expr
#include <tvm/tir/expr.h>
#include <tvm/tir/expr_functor.h>
#include <tvm/tir/function.h>  // For the class PrimFunc
#include <tvm/tir/stmt.h>
#include <tvm/tir/stmt_functor.h>
#include <tvm/tir/transform.h>  // For the decl of the function returning the pass

#include <algorithm>  // For the algorithm std::find
#include <iostream>
#include <string>
#include <unordered_map>  // For the hashtable datatype
#include <utility>        // For std::pair and std::move
#include <vector>

#include "../../printer/text_printer.h"
#include "../analysis/check_contains.h"  // For the visitor CheckContains
#include "common_subexpr_elim_tools.h"   // For the auxiliary analysis (visitors) and tools
#include "replace_selected_expr.h"       // For the mutator ReplaceSelectedExpr

namespace tvm {
namespace tir {

/*!
 * \brief Check whether a computation is forbidden for being treated by the CSE pass.
          The important thing about forbidden computations is that not only we won't want
          to collect them for the CSE pass, but we also won't even want to collect computations
          that contain them.
          The reason is that reusing such computations would change the semantics of the program,
          and therefore before doing any introduction of var or any reuse of already introduced
          variables, we will make sure that the computation being considered is not forbidden, and
          that it does not even contain a forbidden computation.
 * \param expr The expression to check
 * \return Whether `expr` is a forbidden computation or not
 */
bool CommonSubexpressionEliminator::ForbiddenComputation(const PrimExpr& expr) {
  // Function calls, loads and buffer loads are absolutely forbidden as introducing them into
  // variables would change the semantics of the program.
  return (expr.as<CallNode>() != nullptr || expr.as<LoadNode>() != nullptr ||
          expr.as<BufferLoadNode>() != nullptr);
}

/*!
 * \brief Predicate used for verifying that a computation is eligible for being treated by
          the CSE pass, i.e. for being introduced into a variable / for being replaced by a
          variable.
          Being eligible is a conjunction of a few conditions, like not being an atom (constant
          or variable), not being a forbidden node, not containing a forbidden node, etc.
 * \param expr The expression to check
 * \return Whether `expr` is an eligible computation or not
 */
bool CommonSubexpressionEliminator::IsEligibleComputation(const PrimExpr& expr) {
  return (
      // In order to be eligible, the given expression should not be a constant
      (expr.as<IntImmNode>() == nullptr) && (expr.as<FloatImmNode>() == nullptr) &&
      (expr.as<StringImmNode>() == nullptr)
      // and it should not be a variable
      && (expr.as<VarNode>() == nullptr)
      // and it should not be a forbidden computation (function calls and loads)
      && (!ForbiddenComputation(expr))
      // and it should not even contain a forbidden computation (function calls and loads)
      // the reason is that we don't want to register expressions like (x + f(y)) or
      // (x + Mem[i]) as introducing them into variables could change the semantics
      && (!CheckContains::ExprContains(expr, ForbiddenComputation))
      // and it should not be a ramp node or a broadcast node due to some internals TVM
      // constraints (which check for these node explicitely without performing any
      // evaluation first, so if they have been put into variables it fails)
      && (expr.as<RampNode>() == nullptr) && (expr.as<BroadcastNode>() == nullptr));
}

/*!
 * \brief Predicate used (when considering eligible computations) for only diving into
          expressions that are allowed to contain eligible computations. Customize this predicate
          if you want to make it forbidden to rewrite inside a specific node, like inside
          a Load node for instance.
 * \param expr The expression to check
 * \return Whether `expr` can contain some eligible computations or not, and therefore
             if recursing inside `expr` is necessary.
 */
bool CommonSubexpressionEliminator::CanContainEligibleComputations(const PrimExpr& expr) {
  // Uncomment the next line to prevent the collection and the replacement of eligible computations
  // inside the index of Load nodes. We initially thought that this would be needed in order to
  // not harm the indexing mode of the CPU, but as we are still far from ASM code, we
  // finally want to perform such simplifications, which tend to happen fairly frequently.

  // return ( (expr.as<LoadNode>() == nullptr) && (expr.as<BufferLoadNode>() == nullptr) )
  return true;
}

/*!
 * \brief Implements an order on pairs (expression,frequency). First attempts to compare them
          using the size of the expression. If it is the same, decides something else still
          deterministic.
 * \param a The first pair
 * \param b The second pair
 * \return A boolean telling if the first pair `a` comes before the second pair `b`
 * \note We need this order to be deterministic in order to have a fully deterministic pass,
 *       as we will deal with elements that are coming from a hashtable, but the order in which
 *       they appeared in the hashtable was based on some runtime addresses, so it can potentially
 *       change with every execution.
 */
bool CommonSubexpressionEliminator::OrderOnExprAndFrequency(std::pair<PrimExpr, size_t> a,
                                                            std::pair<PrimExpr, size_t> b) {
  size_t a_size = CalculateExprComplexity(a.first);
  size_t b_size = CalculateExprComplexity(b.first);

  // Criteria 1 - Size of the expression comes first
  // `a` comes before `b` if the size of `a` is bigger
  if (a_size > b_size) {
    return true;
  }
  // `a` does NOT come before `b` if the size of `b` is bigger
  if (b_size > a_size) {
    return false;
  }

  // Criteria 2 - If they had the same size, use the lexicographic order as a last resort
  // as we need a deterministic order
  std::stringstream a_stream;
  std::stringstream b_stream;
  a_stream << a.first;
  b_stream << b.first;
  return (a_stream.str().compare(b_stream.str()) < 0);
}

/*!
 * \brief Generates a new fresh variable, whose name will be cse_var_i.
 * \param type_annotation The type of the new variable to generate
 * \return A new variable of type `type_annotation` called cse_var_i where i is the first available
            integer.
 */
Var CommonSubexpressionEliminator::GenerateNewVar(DataType type_annotation) {
  // Increase `num_last_try_` for this new attempt
  num_last_try_++;
  // Builds the variable name, which is sce_var_i where i will go up from 1
  std::string prefix = "cse_var_";
  std::string name = prefix.append(std::to_string(num_last_try_));
  // Builds a String using the std::string
  String string_name(name);

  // Check that the name that we want to use for the new variable isn't already being used
  // (names don't really have to be unique as they are just hints, and having the same name
  // doesn't means that it's the same variable, but it's clearer for dumps)
  if (UsesVarName::StmtUsesVarName(initial_body_, string_name)) {
    // If the name is already used, call ourselves recursively for trying with the next one
    return GenerateNewVar(type_annotation);
  }

  // Increase `nb_var_` for this new generation of variable that we have just done
  nb_var_++;

  // Return a new Variable using the name built and the given type_annotation
  return (Var(string_name, type_annotation));
}

/*!
 * \brief Gives the number of variables generated by the CSE on the current function
           (i.e., getter for `nb_var_`).
 * \return A copy of `nb_var_`
 */
int CommonSubexpressionEliminator::GetNbVarGenerated() { return nb_var_; }

/*!
 * \brief Toplevel (static) method that performs Common Subexpression Elimination on
          a given statement (which should be the body of a PrimFunc). This method should be
          called for each PrimFunc definition.
 * \param stmt The statement of the function being analyzed, on which we want to perform CSE
 * \param context_init The initial context, which should contain the formal parameters
                          of the function being analyzed
 * \return A new statement where CSE has been performed
 */
Stmt CommonSubexpressionEliminator::PerformCSE(const Stmt& stmt, const Context& context_init,
                                               bool identify_equiv_terms) {
  // As this function is being called for each PrimFunc definition, we create a new instance
  // for the one we are having now.
  CommonSubexpressionEliminator common_subexpression_eliminator(stmt, context_init,
                                                                identify_equiv_terms);
  return common_subexpression_eliminator.VisitStmt(stmt);
}

/*!
 * \brief Protected constructor of CommonSubexpressionEliminator.
 * \param context_init The context at the beginning of the CSE pass. It should contain the
                        formal parameters of the function that will be analyzed
 */
CommonSubexpressionEliminator::CommonSubexpressionEliminator(const Stmt& stmt,
                                                             const Context& context_init,
                                                             bool identify_equiv_terms)
    : initial_body_(stmt), context_(context_init), identify_equiv_terms_(identify_equiv_terms) {}

/*!
 * \brief The method which overrides the generic dispatcher of StmtExprMutator.
          Entry point to the common subexpression elimination mutator for expressions.
 * \param expr The expression to mutate
 */
PrimExpr CommonSubexpressionEliminator::VisitExpr(const PrimExpr& expr) {
  bool variables_created = false;  // Will be needed for knowing if the CSE has created new vars
  PrimExpr result = expr;

  // Obtain the (syntactic) eligible computations done by the input expression, and keep it as
  // a ComputationTable, which is a mapping from PrimExpr to size_t, where the size_t is the
  // number of time this exact syntactic computation is being computed.
  ComputationTable table_syntactic_comp_done_by_expr = ComputationsDoneBy::GetComputationsDoneBy(
      expr, IsEligibleComputation, CanContainEligibleComputations);

  // Transform the hashtable of *syntactic* eligible computations into a vector of pairs
  // containing *semantic* entities, i.e. where equivalent computations are merged.
  std::vector<std::pair<PrimExpr, size_t>> semantic_comp_done_by_expr =
      SyntacticToSemanticComputations(table_syntactic_comp_done_by_expr, identify_equiv_terms_);

  // Sort the vector of semantic entities by decreasing size
  std::sort(semantic_comp_done_by_expr.begin(), semantic_comp_done_by_expr.end(),
            OrderOnExprAndFrequency);

  // For each computation done (considering them from biggest to smallest)
  for (size_t i = 0; i < semantic_comp_done_by_expr.size(); i++) {
    std::pair<PrimExpr, size_t>& computation_and_nb = semantic_comp_done_by_expr[i];

    bool ident_equiv_terms = identify_equiv_terms_;  // To avoid the capture of "this"

    // The predicate later used (when doing replacements) to select expressions that are
    // equivalent to the current computation (`computation_and_nb.first`)
    std::function<bool(const PrimExpr&)> predicate_selector =
        [computation_and_nb, ident_equiv_terms](const PrimExpr& current_expr) {
          // `current_expr` should be equivalent to `computation_and_nb.first`, but we also check
          // that `current_expr` is an eligible computation even if we know that
          // `computation_and_nb.first` is eligible by construction, in case that one day the
          // equivalence relation would not preserve the eligibility any more (even though that
          // would probably be a very weird equivalence).
          return (EquivalentTerms(current_expr, computation_and_nb.first, ident_equiv_terms) &&
                  IsEligibleComputation(current_expr));
        };

    // See if there is a pair (`var`, `value`) in the context where `value` is semantically
    // equivalent to `computation_and_nb.first`
    auto it_on_var = std::find_if(
        context_.begin(), context_.end(),
        [computation_and_nb, ident_equiv_terms](const std::pair<Var, MaybeValue>& var_and_value) {
          // Note : safe to call value() as we check has_value() just before
          return (var_and_value.second.has_value() &&
                  EquivalentTerms(var_and_value.second.value(), computation_and_nb.first,
                                  ident_equiv_terms));
        });

    // Case where we have a perfectly equivalent computation already available in a variable
    // introduced (i.e, present in context_).
    // Note that this case is needed when the user has written something like
    // [let x = A in ....A...A...] : we need to be able to replace all the occurrences of A by
    // an already existing variable holding A, when such a variable happens to exist.
    if (it_on_var != context_.end()) {
      // Replace in the current `result` everything that is selected by the selector with
      // the existing variable, without diving into expressions in which we don't have the
      // right to dive.
      result = ReplaceSelectedExpr::ReplaceSelectedExprInExpr(
          result, predicate_selector, it_on_var->first, CanContainEligibleComputations);
    } else {
      // The current computation is not equivalent to a computation already done. We will
      // need to see if we want to introduce it.

      // --- Chunk needed for reusing the UndefinedVars() analysis ---
      // 1 - Wraps the computation into a statement
      Stmt computation_wrapped_in_stmt = Evaluate(computation_and_nb.first);
      // 2.1 - Transform the context into a vector of variables instead of pairs
      std::function<Var(const std::pair<Var, MaybeValue>&)> forget_value =
          [](const std::pair<Var, MaybeValue>& pair) { return pair.first; };
      std::vector<Var> vector_vars_known = VectorMap(context_, forget_value);
      // 2.2 - Transform the std::vector into an Array
      Array<Var> array_vars_known = Array<Var>(vector_vars_known);
      // --- End of chunk needed for reusing the UndefinedVars() analysis ---

      // We use the UndefinedVars() analysis to get the undefined vars of the computation
      Array<Var> vars_undefined = UndefinedVars(computation_wrapped_in_stmt, array_vars_known);

      // Check if we can introduce it : if it contains no undefined variables and if we want
      // to introduce it according to the predicate
      if (vars_undefined.empty() &&
          PredicateIntroVarForComputation(computation_and_nb.first, computation_and_nb.second)) {
        // Create a new variable for this computation
        Var new_var = GenerateNewVar(computation_and_nb.first.dtype());
        // Replace in the current `result` everything that is selected by the selector with
        // the new variable, without diving into expressions in which we don't have the
        // right to dive.
        result = ReplaceSelectedExpr::ReplaceSelectedExprInExpr(result, predicate_selector, new_var,
                                                                CanContainEligibleComputations);
        // Build a let-in that introduces the new variable in the current `result`
        result = Let(new_var, computation_and_nb.first, result);
        // We don't add the variable to the context because the invariant is that the
        // context is the context in which 'result' makes sense, and we've just updated it.
      } else {
        // Here it's not doable to introduce (via a let-in) the computation at this level
        // as it contains variables that are not yet declared, and/or because the predicate
        // did not select it.
        // Either way, we will simply add to the vector of computations the direct subexprs
        // of the current computation, as these ones might be good candidates
        // for being introduced into variables.
        // Note that we don't need to add all of its subexpressions, but only its *direct*
        // subexpressions as we consider them from biggest to smallest, and if they were
        // all added at once, then there could be dependencies between them, as commoning
        // one of them could remove some other possibilities.

        // Computing the direct subexpressions will return a small number of direct
        // subexpressions (typically 0 to 3)
        std::vector<PrimExpr> direct_subexprs = DirectSubexpr::GetDirectSubexpressions(
            computation_and_nb.first, IsEligibleComputation, CanContainEligibleComputations);
        // The following insertion will maintain `semantic_comp_done_by_expr` sorted (by
        // decreasing size/complexity), and it will only insert at locations > i as the
        // direct subexprs are necessarily smaller than the current computation.
        InsertVectorToSortedSemanticComputations(&semantic_comp_done_by_expr, direct_subexprs,
                                                 identify_equiv_terms_);
      }
    }
    // Note : we do not remove the current element, as we never look back in the local vector
  }  // End of for loop

  // If the CSE pass has created some variables, then we run it again as more commoning could
  // potentially happen using the new variables introduced
  if (variables_created) {
    result = VisitExpr(result);
  } else {
    // But if no changes were performed, we recurse inside the children by calling the dispatcher.
    // Calling the dispatcher to the specific treatments, which will update the context
    // appropriately before doing the recursive calls on the children nodes
    result = StmtExprMutator::VisitExpr(result);
  }

  return result;
}

/*!
 * \brief The method which overrides the specific treatment for a LetNode
 */
PrimExpr CommonSubexpressionEliminator::VisitExpr_(const LetNode* op) {
  // At this point, we have already done the generic treatment of introducing (via let-in) what
  // was doable at the toplevel of the given let-in.

  // Save the context at the entry of the function
  Context context_at_entry = context_;

  // Recurse on the `value` field for potentially rewriting it
  PrimExpr value_new = VisitExpr(op->value);

  // Augment the context with the association (`var`, `value`) for preparing the next recursion
  // on the `body`
  context_.push_back({op->var, MaybeValue(op->value)});

  // Recurse on the `body` (with this extended context)
  // The recursive call will have potentially done new simplifications, because in this recursive
  // call `var` will be a part of the context.
  // (see in VisitExpr() that no introduction were performed when a computation was using an
  // undefined variable, as that would lead to ill-formed code)
  PrimExpr body_new = VisitExpr(op->body);

  // Restaure the context to its content at the entrance to not carry out of scope declarations
  // as the variable introduced by the let-in is not in scope outside of its body
  context_ = context_at_entry;

  // Rebuild the let-in with a new `value_new` and `body_new` where new simplifications might
  // have been done.

  // If the `value` and the `body` of the let-in have been rewritten to the same thing
  if (value_new.same_as(op->value) && body_new.same_as(op->body)) {
    // then return a reference to the same node
    return GetRef<PrimExpr>(op);
  } else {
    // Otherwise return a let-in built with the new `value_new` and the new `body_new` that
    // have just been obtained
    return Let(op->var, value_new, body_new, op->span);
  }
}

/*!
 * \brief The method which overrides the generic dispatcher of StmtExprMutator.
          Entry point to the common subexpression elimination mutator for statements.
 * \param stmt The statement to mutate.
 */
Stmt CommonSubexpressionEliminator::VisitStmt(const Stmt& stmt) {
  bool variables_created = false;  // Will be needed for knowing if the CSE has created new vars
  Stmt result = stmt;

  // Obtain the (syntactic) eligible computations done by the input statement, and keep it as
  // a ComputationTable, which is a mapping from PrimExpr to size_t, where the size_t is the
  // number of time this exact syntactic computation is being computed.
  ComputationTable table_syntactic_comp_done_by_stmt = ComputationsDoneBy::GetComputationsDoneBy(
      stmt, IsEligibleComputation, CanContainEligibleComputations);

  // Transform the hashtable of *syntactic* eligible computations into a vector of pairs
  // containing *semantic* entities, i.e. where equivalent computations are merged.
  std::vector<std::pair<PrimExpr, size_t>> semantic_comp_done_by_stmt =
      SyntacticToSemanticComputations(table_syntactic_comp_done_by_stmt, identify_equiv_terms_);

  // Sort the vector of semantic entities by decreasing size
  std::sort(semantic_comp_done_by_stmt.begin(), semantic_comp_done_by_stmt.end(),
            OrderOnExprAndFrequency);

  // For each computation done (considering them from biggest to smallest)
  for (size_t i = 0; i < semantic_comp_done_by_stmt.size(); i++) {
    std::pair<PrimExpr, size_t>& computation_and_nb = semantic_comp_done_by_stmt[i];

    bool ident_equiv_terms = identify_equiv_terms_;  // To avoid the capture of "this"

    // The predicate later used (when doing replacements) to select expressions that are
    // equivalent to the current computation (`computation_and_nb.first`)
    std::function<bool(const PrimExpr&)> predicate_selector =
        [computation_and_nb, ident_equiv_terms](const PrimExpr& current_expr) {
          // `current_expr` should be equivalent to `computation_and_nb.first`, but we also check
          // that `current_expr` is an eligible computation even if we know that
          // `computation_and_nb.first` is eligible by construction, in case that one day the
          // equivalence relation would not preserve the eligibility any more (even though that
          // would probably be a very weird equivalence).
          return (EquivalentTerms(current_expr, computation_and_nb.first, ident_equiv_terms) &&
                  IsEligibleComputation(current_expr));
        };

    // See if there is a pair (`var`, `value`) in the context where `value` is semantically
    // equivalent to `computation_and_nb.first`
    auto it_on_var = std::find_if(
        context_.begin(), context_.end(),
        [computation_and_nb, ident_equiv_terms](const std::pair<Var, MaybeValue>& var_and_value) {
          // Note : safe to call value() as we check has_value() just before
          return (var_and_value.second.has_value() &&
                  EquivalentTerms(var_and_value.second.value(), computation_and_nb.first,
                                  ident_equiv_terms));
        });

    // Case where we have a perfectly equivalent computation already available in a variable
    // introduced (i.e, present in context_).
    // Note that this case is needed when the user has written something like
    // [let x = A in ....A...A...] : we need to be able to replace all the occurrences of A by
    // an already existing variable holding A, when such a variable happens to exist.
    if (it_on_var != context_.end()) {
      // Replace in the current `result` everything that is selected by the selector with
      // the existing variable, without diving into expressions in which we don't have the
      // right to dive.
      result = ReplaceSelectedExpr::ReplaceSelectedExprInStmt(
          result, predicate_selector, it_on_var->first, CanContainEligibleComputations);
    } else {
      // The current computation is not equivalent to a computation already done. We will
      // need to see if we want to introduce it.

      // --- Chunk needed for reusing the UndefinedVars() analysis ---
      // 1 - Wraps the computation into a statement
      Stmt computation_wrapped_in_stmt = Evaluate(computation_and_nb.first);
      // 2.1 - Transform the context into a vector of variables instead of pairs
      std::function<Var(const std::pair<Var, MaybeValue>&)> forget_value =
          [](const std::pair<Var, MaybeValue>& pair) { return pair.first; };
      std::vector<Var> vector_vars_known = VectorMap(context_, forget_value);
      // 2.2 - Transform the std::vector into an Array
      Array<Var> array_vars_known = Array<Var>(vector_vars_known);
      // --- End of chunk needed for reusing the UndefinedVars() analysis ---

      // We use the UndefinedVars() analysis to get the undefined vars of the computation
      Array<Var> vars_undefined = UndefinedVars(computation_wrapped_in_stmt, array_vars_known);

      // Check if we can introduce it : if it contains no undefined variables and if we want
      // to introduce it according to the predicate
      if (vars_undefined.empty() &&
          PredicateIntroVarForComputation(computation_and_nb.first, computation_and_nb.second)) {
        // Create a new variable for this computation
        Var new_var = GenerateNewVar(computation_and_nb.first.dtype());
        variables_created = true;
        // Replace in the current `result` everything that is selected by the selector with
        // the new variable, without diving into expressions in which we don't have the
        // right to dive.
        result = ReplaceSelectedExpr::ReplaceSelectedExprInStmt(result, predicate_selector, new_var,
                                                                CanContainEligibleComputations);
        // Build a let-in that introduces the new variable in the current `result`
        result = LetStmt(new_var, computation_and_nb.first, result);
        // We don't add the variable to the context because the invariant is that the
        // context is the context in which 'result' makes sense, and we've just updated it.
      } else {
        // Here it's not doable to introduce (via a let-in) the computation at this level
        // as it contains variables that are not yet declared, and/or because the predicate
        // did not select it.
        // Either way, we will simply add to the vector of computations the direct subexprs
        // of the current computation, as these ones might be good candidates
        // for being introduced into variables.
        // Note that we don't need to add all of its subexpressions, but only its *direct*
        // subexpressions as we consider them from biggest to smallest, and if they were
        // all added at once, then there could be dependencies between them, as commoning
        // one of them could remove some other possibilities.

        // Computing the direct subexpressions will return a small number of direct
        // subexpressions (typically 0 to 3)
        std::vector<PrimExpr> direct_subexprs = DirectSubexpr::GetDirectSubexpressions(
            computation_and_nb.first, IsEligibleComputation, CanContainEligibleComputations);
        // The following insertion will maintain `semantic_comp_done_by_stmt` sorted (by
        // decreasing size/complexity), and it will only insert at locations > i as the
        // direct subexprs are necessarily smaller than the current computation.
        InsertVectorToSortedSemanticComputations(&semantic_comp_done_by_stmt, direct_subexprs,
                                                 identify_equiv_terms_);
      }
    }
    // Note : we do not remove the current element, as we never look back in the local vector
  }  // End of for loop

  // If the CSE pass has created some variables, then we run it again as more commoning could
  // potentially happen using the new variables introduced
  if (variables_created) {
    result = VisitStmt(result);
  } else {
    // But if no changes were performed, we recurse inside the children by calling the dispatcher.
    // Calling the dispatcher to the specific treatments, which will update the context
    // appropriately before doing the recursive calls on the children nodes
    result = StmtExprMutator::VisitStmt(result);
  }

  return result;
}

/*!
 * \brief The method which overrides the specific treatment for a LetStmtNode
 */
Stmt CommonSubexpressionEliminator::VisitStmt_(const LetStmtNode* op) {
  // At this point, we have already done the generic treatment of introducing (via let-in) what
  // was doable at the toplevel of the given let-in.

  // Save the context at the entry of the function
  Context context_at_entry = context_;

  // Recurse on the `value` field for potentially rewriting it
  PrimExpr value_new = VisitExpr(op->value);

  // Augment the context with the association (`var`, `value`) for preparing the next recursion
  // on the `body`
  context_.push_back({op->var, MaybeValue(op->value)});

  // Recurse on the `body` (with this extended context)
  // The recursive call will have potentially done new simplifications, because in this recursive
  // call `var` will be a part of the context.
  // (see in VisitStmt() that no introduction were performed when a computation was using an
  // undefined variable, as that would lead to ill-formed code)
  Stmt body_new = VisitStmt(op->body);

  // Restaure the context to its content at the entrance to not carry out of scope declarations
  // as the variable introduced by the let-in is not in scope outside of its body
  context_ = context_at_entry;

  // Rebuild the let-in with a new `value_new` and `body_new` where new simplifications might
  // have been done.

  // If the `value` and the `body` of the let-in have been rewritten to the same thing
  if (value_new.same_as(op->value) && body_new.same_as(op->body)) {
    // Return a reference to the same node
    return GetRef<Stmt>(op);
  } else {
    // Otherwise return a let-in built with the new `value_new` and the new `body_new` that
    // have just been obtained
    return LetStmt(op->var, value_new, body_new, op->span);
  }
}

/*!
 * \brief The method which overrides the specific treatment for a ForNode
 */
Stmt CommonSubexpressionEliminator::VisitStmt_(const ForNode* op) {
  // At this point, we have already done the generic treatment of introducing (via let-in) what
  // was doable at the toplevel of the given for loop.

  // Save the context at the entry of the function
  Context context_at_entry = context_;

  // Recurse on the `min` field for potentially rewriting it
  PrimExpr min_new = VisitExpr(op->min);

  // Recurse on the `extent` field for potentially rewriting it
  PrimExpr extent_new = VisitExpr(op->extent);

  // Augment the context with the association {loop_var, no value} (no value as its value will
  // change during the execution of the loop) for preparing the next recursion on the `body`
  context_.push_back({op->loop_var, MaybeValue()});

  // Recurse on the `body` (with this extended context)
  Stmt body_new = VisitStmt(op->body);

  // Restaure the context to its content at the entrance to not carry out of scope declarations
  // as the variable introduced by the for loop is not in scope outside of its body
  context_ = context_at_entry;

  // Rebuild the for loop with (potentially) a new `min_new`, `extent_new` and `body_new`, where
  // new simplifications might have been done.

  // If the `min`, `extent` and `body` of the for loop have been rewritten to the same thing
  if (min_new.same_as(op->min) && extent_new.same_as(op->extent) && body_new.same_as(op->body)) {
    // Return a reference to the same node
    return GetRef<Stmt>(op);
  } else {
    // Otherwise return a for node built with the new `min_new`, `extent_new` and `body_new`
    // that have just been obtained
    return For(op->loop_var, min_new, extent_new, op->kind, body_new, op->thread_binding,
               op->annotations, op->span);
  }
}

Stmt DebugInfoInstaller::InstallInfo(const Stmt& stmt) {
  DebugInfoInstaller installer(stmt);
  return installer.VisitStmt(stmt);
}

DebugInfoInstaller::DebugInfoInstaller(const Stmt& stmt) : initial_body_(stmt) {
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
  // std::cout << "Visiting assert statement at " << op << "\n";
  auto new_stmt = StmtExprMutator::VisitStmt_(op);
  auto newop = new_stmt.as<AssertStmtNode>();
  return AssertStmt(newop->condition, newop->message, newop->body, MaybeSpan(op));
  // // return AssertStmt(newop->condition, newop->message, newop->body,
  // // Span(SourceName::Get("missing-file"), 1, 2, 3, 4)); new_stmt.span
  // auto entry = stmt_lines_.find(op);
  // if (entry == stmt_lines_.end()) {
  //   return AssertStmt(newop->condition, newop->message, newop->body,
  //                     Span(SourceName::Get("missing-file"), 1, 2, 3, 4));
  // } else {
  //   size_t column = 0;
  //   return AssertStmt(
  //       newop->condition, newop->message, newop->body,
  //       Span(SourceName::Get("output.irmodule"), entry->second, entry->second, column, column));
  // }
  // // AssertStmt(PrimExpr condition, PrimExpr message, Stmt body, Span span = Span());
}

namespace transform {

/*!
 * \brief The function which returns the pass for the Common Subexpression Elimination.
 * \return The pass for performing CSE.
 */
Pass CommonSubexprElimTIR(bool enable_cse_tir, bool identify_equiv_terms) {
  auto pass_func = [enable_cse_tir, identify_equiv_terms](PrimFunc f, IRModule m, PassContext ctx) {
    if (enable_cse_tir) {
      auto* n = f.CopyOnWrite();
      Context context_init;
      // Add to the initial context all the parameters of the function, as that is needed for
      // doing commoning on terms that use these parameters (it is only possible to introduce
      // a term into a new variable at a specific point in the program if all the variables that
      // it uses have already been declared at this point)
      for (auto current_param : f->params) {
        // The parameters of the functions are variables associated with no value
        context_init.push_back({current_param, MaybeValue()});
      }

      // Do the Common Subexpression Elimination on the body of the function, with the initial
      // context that we have prepared
      n->body = CommonSubexpressionEliminator::PerformCSE(std::move(f->body), context_init,
                                                          identify_equiv_terms);
    }

    return f;
  };
  return CreatePrimFuncPass(pass_func, 0, "tir.CommonSubexprElimTIR", {});
}

Pass InstallDebugSpans(bool just_dump) {
  auto pass_func = [just_dump](PrimFunc f, IRModule m, PassContext ctx) {
    if (just_dump) {
      // Hacky way to make another pass, delete this
      std::cout << "dumping afterwards\n";
      // These seem to be mostly the same as TIRTextPrinter but with a changed header (?)
      // std::cout << m << "\n";
      // std::cout << f << "\n";
      TextMetaDataContext meta;
      tvm::tir::TIRTextPrinter printer(false, &meta);
      std::cout << printer.Print(f).str() << "\n";

      return f;
    } else {
      std::cout << "running info installer\n";
      auto* n = f.CopyOnWrite();
      n->body = DebugInfoInstaller::InstallInfo(std::move(f->body));

      return f;
    }
  };
  return CreatePrimFuncPass(pass_func, 0, "tir.InstallDebugSpans", {});
}

// The pass can now be invoked via the pass infrastructure, but we also add a Python binding for it
TVM_REGISTER_GLOBAL("tir.transform.CommonSubexprElimTIR").set_body_typed(CommonSubexprElimTIR);

}  // namespace transform
}  // namespace tir
}  // namespace tvm
