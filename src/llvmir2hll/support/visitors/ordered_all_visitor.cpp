/**
* @file src/llvmir2hll/support/visitors/ordered_all_visitor.cpp
* @brief Implementation of OrderedAllVisitor.
* @copyright (c) 2017 Avast Software, licensed under the MIT license
*/

#include "retdec/llvmir2hll/ir/add_op_expr.h"
#include "retdec/llvmir2hll/ir/address_op_expr.h"
#include "retdec/llvmir2hll/ir/and_op_expr.h"
#include "retdec/llvmir2hll/ir/array_index_op_expr.h"
#include "retdec/llvmir2hll/ir/array_type.h"
#include "retdec/llvmir2hll/ir/assign_op_expr.h"
#include "retdec/llvmir2hll/ir/assign_stmt.h"
#include "retdec/llvmir2hll/ir/bit_and_op_expr.h"
#include "retdec/llvmir2hll/ir/bit_cast_expr.h"
#include "retdec/llvmir2hll/ir/bit_or_op_expr.h"
#include "retdec/llvmir2hll/ir/bit_shl_op_expr.h"
#include "retdec/llvmir2hll/ir/bit_shr_op_expr.h"
#include "retdec/llvmir2hll/ir/bit_xor_op_expr.h"
#include "retdec/llvmir2hll/ir/break_stmt.h"
#include "retdec/llvmir2hll/ir/call_expr.h"
#include "retdec/llvmir2hll/ir/call_stmt.h"
#include "retdec/llvmir2hll/ir/comma_op_expr.h"
#include "retdec/llvmir2hll/ir/const_array.h"
#include "retdec/llvmir2hll/ir/const_bool.h"
#include "retdec/llvmir2hll/ir/const_float.h"
#include "retdec/llvmir2hll/ir/const_int.h"
#include "retdec/llvmir2hll/ir/const_null_pointer.h"
#include "retdec/llvmir2hll/ir/const_string.h"
#include "retdec/llvmir2hll/ir/const_struct.h"
#include "retdec/llvmir2hll/ir/const_symbol.h"
#include "retdec/llvmir2hll/ir/continue_stmt.h"
#include "retdec/llvmir2hll/ir/deref_op_expr.h"
#include "retdec/llvmir2hll/ir/div_op_expr.h"
#include "retdec/llvmir2hll/ir/empty_stmt.h"
#include "retdec/llvmir2hll/ir/eq_op_expr.h"
#include "retdec/llvmir2hll/ir/expression.h"
#include "retdec/llvmir2hll/ir/ext_cast_expr.h"
#include "retdec/llvmir2hll/ir/float_type.h"
#include "retdec/llvmir2hll/ir/for_loop_stmt.h"
#include "retdec/llvmir2hll/ir/fp_to_int_cast_expr.h"
#include "retdec/llvmir2hll/ir/function.h"
#include "retdec/llvmir2hll/ir/function_type.h"
#include "retdec/llvmir2hll/ir/global_var_def.h"
#include "retdec/llvmir2hll/ir/goto_stmt.h"
#include "retdec/llvmir2hll/ir/gt_eq_op_expr.h"
#include "retdec/llvmir2hll/ir/gt_op_expr.h"
#include "retdec/llvmir2hll/ir/if_stmt.h"
#include "retdec/llvmir2hll/ir/int_to_fp_cast_expr.h"
#include "retdec/llvmir2hll/ir/int_to_ptr_cast_expr.h"
#include "retdec/llvmir2hll/ir/int_type.h"
#include "retdec/llvmir2hll/ir/lt_eq_op_expr.h"
#include "retdec/llvmir2hll/ir/lt_op_expr.h"
#include "retdec/llvmir2hll/ir/mod_op_expr.h"
#include "retdec/llvmir2hll/ir/mul_op_expr.h"
#include "retdec/llvmir2hll/ir/neg_op_expr.h"
#include "retdec/llvmir2hll/ir/neq_op_expr.h"
#include "retdec/llvmir2hll/ir/not_op_expr.h"
#include "retdec/llvmir2hll/ir/or_op_expr.h"
#include "retdec/llvmir2hll/ir/pointer_type.h"
#include "retdec/llvmir2hll/ir/ptr_to_int_cast_expr.h"
#include "retdec/llvmir2hll/ir/return_stmt.h"
#include "retdec/llvmir2hll/ir/statement.h"
#include "retdec/llvmir2hll/ir/string_type.h"
#include "retdec/llvmir2hll/ir/struct_index_op_expr.h"
#include "retdec/llvmir2hll/ir/struct_type.h"
#include "retdec/llvmir2hll/ir/sub_op_expr.h"
#include "retdec/llvmir2hll/ir/switch_stmt.h"
#include "retdec/llvmir2hll/ir/ternary_op_expr.h"
#include "retdec/llvmir2hll/ir/trunc_cast_expr.h"
#include "retdec/llvmir2hll/ir/ufor_loop_stmt.h"
#include "retdec/llvmir2hll/ir/unknown_type.h"
#include "retdec/llvmir2hll/ir/unreachable_stmt.h"
#include "retdec/llvmir2hll/ir/var_def_stmt.h"
#include "retdec/llvmir2hll/ir/variable.h"
#include "retdec/llvmir2hll/ir/void_type.h"
#include "retdec/llvmir2hll/ir/while_loop_stmt.h"
#include "retdec/llvmir2hll/support/debug.h"
#include "retdec/llvmir2hll/support/smart_ptr.h"
#include "retdec/llvmir2hll/support/visitors/ordered_all_visitor.h"
#include "retdec/utils/container.h"
#include "retdec/llvmir2hll/support/manager/visitor_manager.h"

using retdec::utils::hasItem;
using namespace std::placeholders;
namespace retdec {
namespace llvmir2hll {

/**
* @brief Constructs a new visitor.
*/
OrderedAllVisitor::OrderedAllVisitor(bool visitSuccessors, bool visitNestedStmts):
	Visitor(), lastStmt(), accessedStmts(), visitSuccessors(visitSuccessors),
	visitNestedStmts(visitNestedStmts) {}

/**
* @brief Destructs the visitor.
*/
OrderedAllVisitor::~OrderedAllVisitor() {}

void OrderedAllVisitor::visit(ShPtr<GlobalVarDef> varDef) {
	VISIT_THIS(varDef->getVar())
	if (ShPtr<Expression> init = varDef->getInitializer()) {
		VISIT_THIS(init)
	}
}

void OrderedAllVisitor::visit(ShPtr<Function> func) {
	// For each parameter...
	for (const auto &param : func->getParams()) {
		VISIT_THIS(param)
	}

	if (ShPtr<Statement> body = func->getBody()) {
		visitStmt(body);
	}
}

//
// Statements
//

void OrderedAllVisitor::visit(ShPtr<AssignStmt> stmt) {
	lastStmt = stmt;
	VISIT_THIS(stmt->getLhs())
	VISIT_THIS(stmt->getRhs())
	if (visitSuccessors && stmt->hasSuccessor()) {
		visitStmt(stmt->getSuccessor());
	}
}

void OrderedAllVisitor::visit(ShPtr<VarDefStmt> stmt) {
	lastStmt = stmt;
	VISIT_THIS(stmt->getVar())
	if (ShPtr<Expression> init = stmt->getInitializer()) {
		VISIT_THIS(init)
	}
	if (visitSuccessors && stmt->hasSuccessor()) {
		visitStmt(stmt->getSuccessor());
	}
}

void OrderedAllVisitor::visit(ShPtr<CallStmt> stmt) {
	lastStmt = stmt;
	VISIT_THIS(stmt->getCall())
	if (visitSuccessors && stmt->hasSuccessor()) {
		visitStmt(stmt->getSuccessor());
	}
}

void OrderedAllVisitor::visit(ShPtr<ReturnStmt> stmt) {
	lastStmt = stmt;
	if (ShPtr<Expression> retVal = stmt->getRetVal()) {
		VISIT_THIS(retVal)
	}
	if (visitSuccessors && stmt->hasSuccessor()) {
		visitStmt(stmt->getSuccessor());
	}
}

void OrderedAllVisitor::visit(ShPtr<EmptyStmt> stmt) {
	lastStmt = stmt;
	if (visitSuccessors && stmt->hasSuccessor()) {
		visitStmt(stmt->getSuccessor());
	}
}

void OrderedAllVisitor::visit(ShPtr<IfStmt> stmt) {
	lastStmt = stmt;
	// For each clause...
	for (auto i = stmt->clause_begin(), e = stmt->clause_end(); i != e; ++i) {
VISIT_THIS(		i->first);
		if (visitNestedStmts) {
			visitStmt(i->second);
		}
	}

	if (visitNestedStmts && stmt->hasElseClause()) {
		visitStmt(stmt->getElseClause());
	}
	if (visitSuccessors && stmt->hasSuccessor()) {
		visitStmt(stmt->getSuccessor());
	}
}

void OrderedAllVisitor::visit(ShPtr<SwitchStmt> stmt) {
	lastStmt = stmt;
VISIT_THIS(	stmt->getControlExpr());

	// For each clause...
	for (auto i = stmt->clause_begin(), e = stmt->clause_end(); i != e; ++i) {
		if (i->first) {
			VISIT_THIS(i->first)
		}
		if (visitNestedStmts && i->second) {
			visitStmt(i->second);
		}
	}
	if (visitSuccessors && stmt->hasSuccessor()) {
		visitStmt(stmt->getSuccessor());
	}
}

void OrderedAllVisitor::visit(ShPtr<WhileLoopStmt> stmt) {
	lastStmt = stmt;
	VISIT_THIS(stmt->getCondition())
	if (visitNestedStmts) {
		visitStmt(stmt->getBody());
	}
	if (visitSuccessors && stmt->hasSuccessor()) {
		visitStmt(stmt->getSuccessor());
	}
}

void OrderedAllVisitor::visit(ShPtr<ForLoopStmt> stmt) {
	lastStmt = stmt;

	VISIT_THIS(stmt->getIndVar())
	VISIT_THIS(stmt->getStartValue())
	VISIT_THIS(stmt->getEndCond())
	VISIT_THIS(stmt->getStep())
	if (visitNestedStmts) {
		visitStmt(stmt->getBody());
	}
	if (visitSuccessors && stmt->hasSuccessor()) {
		visitStmt(stmt->getSuccessor());
	}
}

void OrderedAllVisitor::visit(ShPtr<UForLoopStmt> stmt) {
	lastStmt = stmt;
	if (auto init = stmt->getInit()) {
		VISIT_THIS(stmt->getInit())
VISIT_THIS(		init);
	}
	if (auto cond = stmt->getCond()) {
		VISIT_THIS(stmt->getCond())
VISIT_THIS(		cond);
	}
	if (auto step = stmt->getStep()) {
		VISIT_THIS(stmt->getStep())
	}
	if (visitNestedStmts) {
		visitStmt(stmt->getBody());
	}
	if (visitSuccessors && stmt->hasSuccessor()) {
		visitStmt(stmt->getSuccessor());
	}
}

void OrderedAllVisitor::visit(ShPtr<BreakStmt> stmt) {
	lastStmt = stmt;
	if (visitSuccessors && stmt->hasSuccessor()) {
		visitStmt(stmt->getSuccessor());
	}
}

void OrderedAllVisitor::visit(ShPtr<ContinueStmt> stmt) {
	lastStmt = stmt;
	if (visitSuccessors && stmt->hasSuccessor()) {
		visitStmt(stmt->getSuccessor());
	}
}

void OrderedAllVisitor::visit(ShPtr<GotoStmt> stmt) {
	lastStmt = stmt;
	if (visitSuccessors && stmt->hasSuccessor()) {
		visitStmt(stmt->getTarget());
		visitStmt(stmt->getSuccessor());
	}
}

void OrderedAllVisitor::visit(ShPtr<UnreachableStmt> stmt) {
	lastStmt = stmt;
	if (visitSuccessors && stmt->hasSuccessor()) {
		visitStmt(stmt->getSuccessor());
	}
}

//
// Expressions
//

void OrderedAllVisitor::visit(ShPtr<AddressOpExpr> expr) {
	VISIT_THIS(expr->getOperand())
}

void OrderedAllVisitor::visit(ShPtr<AssignOpExpr> expr) {
	VISIT_THIS(expr->getFirstOperand())
	VISIT_THIS(expr->getSecondOperand())
}

void OrderedAllVisitor::visit(ShPtr<ArrayIndexOpExpr> expr) {
	VISIT_THIS(expr->getFirstOperand())
	VISIT_THIS(expr->getSecondOperand())
}

void OrderedAllVisitor::visit(ShPtr<StructIndexOpExpr> expr) {
	VISIT_THIS(expr->getFirstOperand())
	VISIT_THIS(expr->getSecondOperand())
}

void OrderedAllVisitor::visit(ShPtr<DerefOpExpr> expr) {
	VISIT_THIS(expr->getOperand())
}

void OrderedAllVisitor::visit(ShPtr<NotOpExpr> expr) {
	VISIT_THIS(expr->getOperand())
}

void OrderedAllVisitor::visit(ShPtr<NegOpExpr> expr) {
	VISIT_THIS(expr->getOperand())
}

void OrderedAllVisitor::visit(ShPtr<EqOpExpr> expr) {
	VISIT_THIS(expr->getFirstOperand())
	VISIT_THIS(expr->getSecondOperand())
}

void OrderedAllVisitor::visit(ShPtr<NeqOpExpr> expr) {
	VISIT_THIS(expr->getFirstOperand())
	VISIT_THIS(expr->getSecondOperand())
}

void OrderedAllVisitor::visit(ShPtr<LtEqOpExpr> expr) {
	VISIT_THIS(expr->getFirstOperand())
	VISIT_THIS(expr->getSecondOperand())
}

void OrderedAllVisitor::visit(ShPtr<GtEqOpExpr> expr) {
	VISIT_THIS(expr->getFirstOperand())
	VISIT_THIS(expr->getSecondOperand())
}

void OrderedAllVisitor::visit(ShPtr<LtOpExpr> expr) {
	VISIT_THIS(expr->getFirstOperand())
	VISIT_THIS(expr->getSecondOperand())
}

void OrderedAllVisitor::visit(ShPtr<GtOpExpr> expr) {
	VISIT_THIS(expr->getFirstOperand())
	VISIT_THIS(expr->getSecondOperand())
}

void OrderedAllVisitor::visit(ShPtr<AddOpExpr> expr) {
	VISIT_THIS(expr->getFirstOperand())
	VISIT_THIS(expr->getSecondOperand())
}

void OrderedAllVisitor::visit(ShPtr<SubOpExpr> expr) {
	VISIT_THIS(expr->getFirstOperand())
	VISIT_THIS(expr->getSecondOperand())
}

void OrderedAllVisitor::visit(ShPtr<MulOpExpr> expr) {
	VISIT_THIS(expr->getFirstOperand())
	VISIT_THIS(expr->getSecondOperand())
}

void OrderedAllVisitor::visit(ShPtr<ModOpExpr> expr) {
	VISIT_THIS(expr->getFirstOperand())
	VISIT_THIS(expr->getSecondOperand())
}

void OrderedAllVisitor::visit(ShPtr<DivOpExpr> expr) {
	VISIT_THIS(expr->getFirstOperand())
	VISIT_THIS(expr->getSecondOperand())
}

void OrderedAllVisitor::visit(ShPtr<AndOpExpr> expr) {
	VISIT_THIS(expr->getFirstOperand())
	VISIT_THIS(expr->getSecondOperand())
}

void OrderedAllVisitor::visit(ShPtr<OrOpExpr> expr) {
	VISIT_THIS(expr->getFirstOperand())
	VISIT_THIS(expr->getSecondOperand())
}

void OrderedAllVisitor::visit(ShPtr<BitAndOpExpr> expr) {
	VISIT_THIS(expr->getFirstOperand())
	VISIT_THIS(expr->getSecondOperand())
}

void OrderedAllVisitor::visit(ShPtr<BitOrOpExpr> expr) {
	VISIT_THIS(expr->getFirstOperand())
	VISIT_THIS(expr->getSecondOperand())
}

void OrderedAllVisitor::visit(ShPtr<BitXorOpExpr> expr) {
	VISIT_THIS(expr->getFirstOperand())
	VISIT_THIS(expr->getSecondOperand())
}

void OrderedAllVisitor::visit(ShPtr<BitShlOpExpr> expr) {
	VISIT_THIS(expr->getFirstOperand())
	VISIT_THIS(expr->getSecondOperand())
}

void OrderedAllVisitor::visit(ShPtr<BitShrOpExpr> expr) {
	VISIT_THIS(expr->getFirstOperand())
	VISIT_THIS(expr->getSecondOperand())
}

void OrderedAllVisitor::visit(ShPtr<TernaryOpExpr> expr) {
	VISIT_THIS(expr->getCondition())
	VISIT_THIS(expr->getTrueValue())
	VISIT_THIS(expr->getFalseValue())
}

void OrderedAllVisitor::visit(ShPtr<CallExpr> expr) {
	VISIT_THIS(expr->getCalledExpr())
VISIT_THIS(	expr->getCalledExpr());

	// For each argument...
	for (const auto &arg : expr->getArgs()) {
		VISIT_THIS(arg)
VISIT_THIS(		arg);
	}
}

void OrderedAllVisitor::visit(ShPtr<CommaOpExpr> expr) {
	VISIT_THIS(expr->getFirstOperand())
	VISIT_THIS(expr->getSecondOperand())
}

void OrderedAllVisitor::visit(ShPtr<Variable> var) {}

//
// Casts
//

void OrderedAllVisitor::visit(ShPtr<BitCastExpr> expr) {
	VISIT_THIS(expr->getOperand())
}

void OrderedAllVisitor::visit(ShPtr<ExtCastExpr> expr) {
	VISIT_THIS(expr->getOperand())
}

void OrderedAllVisitor::visit(ShPtr<TruncCastExpr> expr) {
	VISIT_THIS(expr->getOperand())
}

void OrderedAllVisitor::visit(ShPtr<FPToIntCastExpr> expr) {
	VISIT_THIS(expr->getOperand())
}

void OrderedAllVisitor::visit(ShPtr<IntToFPCastExpr> expr) {
	VISIT_THIS(expr->getOperand())
}

void OrderedAllVisitor::visit(ShPtr<IntToPtrCastExpr> expr) {
	VISIT_THIS(expr->getOperand())
}

void OrderedAllVisitor::visit(ShPtr<PtrToIntCastExpr> expr) {
	VISIT_THIS(expr->getOperand())
}

//
// Constants
//

void OrderedAllVisitor::visit(ShPtr<ConstBool> constant) {}

void OrderedAllVisitor::visit(ShPtr<ConstFloat> constant) {}

void OrderedAllVisitor::visit(ShPtr<ConstInt> constant) {}

void OrderedAllVisitor::visit(ShPtr<ConstNullPointer> constant) {}

void OrderedAllVisitor::visit(ShPtr<ConstString> constant) {}

void OrderedAllVisitor::visit(ShPtr<ConstArray> constant) {
	if (constant->isInitialized()) {
		for (const auto &element : constant->getInitializedValue()) {
			VISIT_THIS(element)
		}
	}
}

void OrderedAllVisitor::visit(ShPtr<ConstStruct> constant) {
	for (const auto &member : constant->getValue()) {
		VISIT_THIS(member.first)
		VISIT_THIS(member.second)
	}
}

void OrderedAllVisitor::visit(ShPtr<ConstSymbol> constant) {
	VISIT_THIS(constant->getValue())
}

//
// Types
//

void OrderedAllVisitor::visit(ShPtr<FloatType> type) {
	if (makeAccessedAndCheckIfAccessed(type)) {
		return;
	}
}

void OrderedAllVisitor::visit(ShPtr<IntType> type) {
	if (makeAccessedAndCheckIfAccessed(type)) {
		return;
	}
}

void OrderedAllVisitor::visit(ShPtr<PointerType> type) {
	VISIT_THIS(type->getContainedType())
	if (makeAccessedAndCheckIfAccessed(type)) {
		return;
	}
}

void OrderedAllVisitor::visit(ShPtr<StringType> type) {
	if (makeAccessedAndCheckIfAccessed(type)) {
		return;
	}
}

void OrderedAllVisitor::visit(ShPtr<ArrayType> type) {
	if (makeAccessedAndCheckIfAccessed(type)) {
		return;
	}
	VISIT_THIS(type->getContainedType())
}

void OrderedAllVisitor::visit(ShPtr<StructType> type) {
	if (makeAccessedAndCheckIfAccessed(type)) {
		return;
	}

	const StructType::ElementTypes &elements(type->getElementTypes());
	for (StructType::ElementTypes::size_type i = 0, e = elements.size(); i < e; ++i) {
		VISIT_THIS(elements[i])
	}
}

void OrderedAllVisitor::visit(ShPtr<FunctionType> type) {
	if (makeAccessedAndCheckIfAccessed(type)) {
		return;
	}

	// Return type.
	VISIT_THIS(type->getRetType())

	// Argument types.
	for (auto i = type->param_begin(), e = type->param_end(); i != e; ++i) {
		VISIT_THIS((*i))
	}
}

void OrderedAllVisitor::visit(ShPtr<VoidType> type) {
	if (makeAccessedAndCheckIfAccessed(type)) {
		return;
	}
}

void OrderedAllVisitor::visit(ShPtr<UnknownType> type) {
	if (makeAccessedAndCheckIfAccessed(type)) {
		return;
	}
}

/**
* @brief Visits the given statement, and possibly its successors or nested
*        statements.
*
* @param[in] stmt Statement to be visited.
* @param[in] visitSuccessors If @c true, a successor of @a stmt is also visited
*                            (and a successor of this successor, and so on).
* @param[in] visitNestedStmts If @c true, nested statements are also visited,
*                             e.g. loop, if, and switch statement's bodies.
*
* If @a stmt has already been accessed, this function does nothing. If @a stmt
* is the null pointer, it also does nothing. Before visiting @a stmt, this
* function adds it to @c accessedStmts.
*/
void OrderedAllVisitor::visitStmt(ShPtr<Statement> stmt, bool visitSuccessors,
		bool visitNestedStmts) {
	if (stmt && !hasItem(accessedStmts, stmt)) {
		this->visitSuccessors = visitSuccessors;
		this->visitNestedStmts = visitNestedStmts;
		accessedStmts.insert(stmt);
		VISIT_THIS(stmt)
	}
}

/**
* @brief "Restarts" the visitor so it is in the state like it was when it was
*        created.
*
* @param[in] visitSuccessors New value of this attribute.
* @param[in] visitNestedStmts New value of this attribute.
*/
void OrderedAllVisitor::restart(bool visitSuccessors, bool visitNestedStmts) {
	accessedStmts.clear();
	accessedTypes.clear();
	this->visitSuccessors = visitSuccessors;
	this->visitNestedStmts = visitNestedStmts;
}

/**
* @brief Makes the given type accessed.
*
* @return @c true if @a type has already been accessed, @c false otherwise.
*/
bool OrderedAllVisitor::makeAccessedAndCheckIfAccessed(ShPtr<Type> type) {
	if (hasItem(accessedTypes, type)) {
		return true;
	}
	accessedTypes.insert(type);
	return false;
}

} // namespace llvmir2hll
} // namespace retdec
