/**
* @file src/llvmir2hll/support/value_text_repr_visitor.cpp
* @brief Implementation of ValueTextReprVisitor.
* @copyright (c) 2017 Avast Software, licensed under the MIT license
*/

#include "retdec/llvmir2hll/ir/add_op_expr.h"
#include "retdec/llvmir2hll/ir/address_op_expr.h"
#include "retdec/llvmir2hll/ir/and_op_expr.h"
#include "retdec/llvmir2hll/ir/array_index_op_expr.h"
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
#include "retdec/llvmir2hll/ir/for_loop_stmt.h"
#include "retdec/llvmir2hll/ir/fp_to_int_cast_expr.h"
#include "retdec/llvmir2hll/ir/function.h"
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
#include "retdec/llvmir2hll/ir/module.h"
#include "retdec/llvmir2hll/ir/mul_op_expr.h"
#include "retdec/llvmir2hll/ir/neg_op_expr.h"
#include "retdec/llvmir2hll/ir/neq_op_expr.h"
#include "retdec/llvmir2hll/ir/not_op_expr.h"
#include "retdec/llvmir2hll/ir/or_op_expr.h"
#include "retdec/llvmir2hll/ir/pointer_type.h"
#include "retdec/llvmir2hll/ir/ptr_to_int_cast_expr.h"
#include "retdec/llvmir2hll/ir/return_stmt.h"
#include "retdec/llvmir2hll/ir/statement.h"
#include "retdec/llvmir2hll/ir/struct_index_op_expr.h"
#include "retdec/llvmir2hll/ir/struct_type.h"
#include "retdec/llvmir2hll/ir/sub_op_expr.h"
#include "retdec/llvmir2hll/ir/switch_stmt.h"
#include "retdec/llvmir2hll/ir/ternary_op_expr.h"
#include "retdec/llvmir2hll/ir/trunc_cast_expr.h"
#include "retdec/llvmir2hll/ir/ufor_loop_stmt.h"
#include "retdec/llvmir2hll/ir/unreachable_stmt.h"
#include "retdec/llvmir2hll/ir/var_def_stmt.h"
#include "retdec/llvmir2hll/ir/variable.h"
#include "retdec/llvmir2hll/ir/while_loop_stmt.h"
#include "retdec/llvmir2hll/support/debug.h"
#include "retdec/llvmir2hll/support/smart_ptr.h"
#include "retdec/llvmir2hll/support/value_text_repr_visitor.h"
#include "retdec/llvmir2hll/support/manager/visitor_manager.h"

namespace retdec {
namespace llvmir2hll {

/**
* @brief Constructs a new visitor.
*/
ValueTextReprVisitor::ValueTextReprVisitor():
	OrderedAllVisitor(), textRepr() {}

/**
* @brief Destructs the visitor.
*/
ValueTextReprVisitor::~ValueTextReprVisitor() {}

/**
* @brief Returns a textual representation of @a value.
*
* @param[in] value Value whose textual representation will be obtained.
*
* The returned representation is a concise representation of @a value in a
* Python-like language. It is meant to be used for debugging and developing
* purposes (for example, you can use it during debugging or when emitting a
* control-flow graph). The returned string is never ended with a new line. No
* successors of statements are considered, i.e. if @a value is a statement,
* only its representation is returned.
*
* If @a value is a compound statement or a function, the bodies of nested
* blocks are not considered. However, in this case, the returned string may
* contain new lines for better readability.
*
* @par Preconditions
*  - @a value is non-null
*
* TODO: Incorporate this functionality into HLL writers? The implementation of
*       this class is based on PyHLLWriter, anyhow.
*/
std::string ValueTextReprVisitor::getTextRepr(ShPtr<Value> value) {
	PRECONDITION_NON_NULL(value);

	ShPtr<ValueTextReprVisitor> visitor(new ValueTextReprVisitor());
	VISIT(value, visitor.get())
	return visitor->textRepr.str();
}

void ValueTextReprVisitor::visit(ShPtr<GlobalVarDef> varDef) {
	VISIT_THIS(varDef->getVar())
	if (ShPtr<Expression> init = varDef->getInitializer()) {
		textRepr << " = ";
		VISIT_THIS(init)
	}
}

void ValueTextReprVisitor::visit(ShPtr<Function> func) {
	textRepr << "def " << func->getName() << "(";

	// For each parameter...
	bool paramEmitted = false;
	for (const auto &param : func->getParams()) {
		if (paramEmitted) {
			textRepr << ", ";
		}
		// If the parameter is a pointer, emit a proper amount of "*"s.
		if (ShPtr<PointerType> type = cast<PointerType>(param->getType())) {
			do {
				textRepr << "*";
			} while ((type = cast<PointerType>(type->getContainedType())));
		}
		VISIT_THIS(param)
		paramEmitted = true;
	}

	// Optional vararg indication.
	if (func->isVarArg()) {
		if (paramEmitted) {
			textRepr << ", ";
		}
		textRepr << "...";
	}

	textRepr << ")";
}

void ValueTextReprVisitor::visit(ShPtr<Variable> var) {
	textRepr << var->getName();
}

void ValueTextReprVisitor::visit(ShPtr<AddressOpExpr> expr) {
	textRepr << "&(";
	VISIT_THIS(expr->getOperand())
	textRepr << ")";
}

void ValueTextReprVisitor::visit(ShPtr<AssignOpExpr> expr) {
	textRepr << "(";
	VISIT_THIS(expr->getFirstOperand())
	textRepr << " = ";
	VISIT_THIS(expr->getSecondOperand())
	textRepr << ")";
}

void ValueTextReprVisitor::visit(ShPtr<ArrayIndexOpExpr> expr) {
	VISIT_THIS(expr->getFirstOperand())
	textRepr << "[";
	VISIT_THIS(expr->getSecondOperand())
	textRepr << "]";
}

void ValueTextReprVisitor::visit(ShPtr<StructIndexOpExpr> expr) {
	VISIT_THIS(expr->getFirstOperand())
	textRepr << "['";
	VISIT_THIS(expr->getSecondOperand())
	textRepr << "']";
}

void ValueTextReprVisitor::visit(ShPtr<DerefOpExpr> expr) {
	textRepr << "*(";
	VISIT_THIS(expr->getOperand())
	textRepr << ")";
}

void ValueTextReprVisitor::visit(ShPtr<NotOpExpr> expr) {
	textRepr << "not (";
	VISIT_THIS(expr->getOperand())
	textRepr << ")";
}

void ValueTextReprVisitor::visit(ShPtr<NegOpExpr> expr) {
	textRepr << "-(";
	VISIT_THIS(expr->getOperand())
	textRepr << ")";
}

void ValueTextReprVisitor::visit(ShPtr<EqOpExpr> expr) {
	textRepr << "(";
	VISIT_THIS(expr->getFirstOperand())
	textRepr << " == ";
	VISIT_THIS(expr->getSecondOperand())
	textRepr << ")";
}

void ValueTextReprVisitor::visit(ShPtr<NeqOpExpr> expr) {
	textRepr << "(";
	VISIT_THIS(expr->getFirstOperand())
	textRepr << " != ";
	VISIT_THIS(expr->getSecondOperand())
	textRepr << ")";
}

void ValueTextReprVisitor::visit(ShPtr<LtOpExpr> expr) {
	textRepr << "(";
	VISIT_THIS(expr->getFirstOperand())
	textRepr << " < ";
	VISIT_THIS(expr->getSecondOperand())
	textRepr << ")";
}

void ValueTextReprVisitor::visit(ShPtr<GtOpExpr> expr) {
	textRepr << "(";
	VISIT_THIS(expr->getFirstOperand())
	textRepr << " > ";
	VISIT_THIS(expr->getSecondOperand())
	textRepr << ")";
}

void ValueTextReprVisitor::visit(ShPtr<LtEqOpExpr> expr) {
	textRepr << "(";
	VISIT_THIS(expr->getFirstOperand())
	textRepr << " <= ";
	VISIT_THIS(expr->getSecondOperand())
	textRepr << ")";
}

void ValueTextReprVisitor::visit(ShPtr<GtEqOpExpr> expr) {
	textRepr << "(";
	VISIT_THIS(expr->getFirstOperand())
	textRepr << " >= ";
	VISIT_THIS(expr->getSecondOperand())
	textRepr << ")";
}

void ValueTextReprVisitor::visit(ShPtr<TernaryOpExpr> expr) {
	textRepr << "(";
	VISIT_THIS(expr->getTrueValue())
	textRepr << " if ";
	VISIT_THIS(expr->getCondition())
	textRepr << " else ";
	VISIT_THIS(expr->getFalseValue())
	textRepr << ")";
}

void ValueTextReprVisitor::visit(ShPtr<AddOpExpr> expr) {
	textRepr << "(";
	VISIT_THIS(expr->getFirstOperand())
	textRepr << " + ";
	VISIT_THIS(expr->getSecondOperand())
	textRepr << ")";
}

void ValueTextReprVisitor::visit(ShPtr<SubOpExpr> expr) {
	textRepr << "(";
	VISIT_THIS(expr->getFirstOperand())
	textRepr << " - ";
	VISIT_THIS(expr->getSecondOperand())
	textRepr << ")";
}

void ValueTextReprVisitor::visit(ShPtr<MulOpExpr> expr) {
	textRepr << "(";
	VISIT_THIS(expr->getFirstOperand())
	textRepr << " * ";
	VISIT_THIS(expr->getSecondOperand())
	textRepr << ")";
}

void ValueTextReprVisitor::visit(ShPtr<ModOpExpr> expr) {
	textRepr << "(";
	VISIT_THIS(expr->getFirstOperand())
	textRepr << " % ";
	VISIT_THIS(expr->getSecondOperand())
	textRepr << ")";
}

void ValueTextReprVisitor::visit(ShPtr<DivOpExpr> expr) {
	textRepr << "(";
	VISIT_THIS(expr->getFirstOperand())
	textRepr << " / ";
	VISIT_THIS(expr->getSecondOperand())
	textRepr << ")";
}

void ValueTextReprVisitor::visit(ShPtr<AndOpExpr> expr) {
	textRepr << "(";
	VISIT_THIS(expr->getFirstOperand())
	textRepr << " and ";
	VISIT_THIS(expr->getSecondOperand())
	textRepr << ")";
}

void ValueTextReprVisitor::visit(ShPtr<OrOpExpr> expr) {
	textRepr << "(";
	VISIT_THIS(expr->getFirstOperand())
	textRepr << " or ";
	VISIT_THIS(expr->getSecondOperand())
	textRepr << ")";
}

void ValueTextReprVisitor::visit(ShPtr<BitAndOpExpr> expr) {
	textRepr << "(";
	VISIT_THIS(expr->getFirstOperand())
	textRepr << " & ";
	VISIT_THIS(expr->getSecondOperand())
	textRepr << ")";
}

void ValueTextReprVisitor::visit(ShPtr<BitOrOpExpr> expr) {
	textRepr << "(";
	VISIT_THIS(expr->getFirstOperand())
	textRepr << " | ";
	VISIT_THIS(expr->getSecondOperand())
	textRepr << ")";
}

void ValueTextReprVisitor::visit(ShPtr<BitXorOpExpr> expr) {
	textRepr << "(";
	VISIT_THIS(expr->getFirstOperand())
	textRepr << " ^ ";
	VISIT_THIS(expr->getSecondOperand())
	textRepr << ")";
}

void ValueTextReprVisitor::visit(ShPtr<BitShlOpExpr> expr) {
	textRepr << "(";
	VISIT_THIS(expr->getFirstOperand())
	textRepr << " << ";
	VISIT_THIS(expr->getSecondOperand())
	textRepr << ")";
}

void ValueTextReprVisitor::visit(ShPtr<BitShrOpExpr> expr) {
	textRepr << "(";
	VISIT_THIS(expr->getFirstOperand())
	textRepr << " >> ";
	VISIT_THIS(expr->getSecondOperand())
	textRepr << ")";
}

void ValueTextReprVisitor::visit(ShPtr<CallExpr> expr) {
	VISIT_THIS(expr->getCalledExpr())
	textRepr << "(";
	// For each argument...
	bool argEmitted = false;
	for (const auto &arg : expr->getArgs()) {
		if (argEmitted) {
			textRepr << ", ";
		}
		VISIT_THIS(arg)
		argEmitted = true;
	}
	textRepr << ")";
}

void ValueTextReprVisitor::visit(ShPtr<CommaOpExpr> expr) {
	textRepr << "(";
	VISIT_THIS(expr->getFirstOperand())
	textRepr << ", ";
	VISIT_THIS(expr->getSecondOperand())
	textRepr << ")";
}

// Casts.
void ValueTextReprVisitor::visit(ShPtr<BitCastExpr> expr) {
	textRepr << "BitCastExpr<";
	VISIT_THIS(expr->getType())
	textRepr << ">(";
	VISIT_THIS(expr->getOperand())
	textRepr << ")";
}

void ValueTextReprVisitor::visit(ShPtr<ExtCastExpr> expr) {
	textRepr << "ExtCastExpr<";
	VISIT_THIS(expr->getType())
	textRepr << ">(";
	VISIT_THIS(expr->getOperand())
	textRepr << ")";
}

void ValueTextReprVisitor::visit(ShPtr<TruncCastExpr> expr) {
	textRepr << "TruncCastExpr<";
	VISIT_THIS(expr->getType())
	textRepr << ">(";
	VISIT_THIS(expr->getOperand())
	textRepr << ")";
}

void ValueTextReprVisitor::visit(ShPtr<FPToIntCastExpr> expr) {
	textRepr << "FPToIntCastExpr<";
	VISIT_THIS(expr->getType())
	textRepr << ">(";
	VISIT_THIS(expr->getOperand())
	textRepr << ")";
}

void ValueTextReprVisitor::visit(ShPtr<IntToFPCastExpr> expr) {
	textRepr << "IntToFPCastExpr<";
	VISIT_THIS(expr->getType())
	textRepr << ">(";
	VISIT_THIS(expr->getOperand())
	textRepr << ")";
}

void ValueTextReprVisitor::visit(ShPtr<IntToPtrCastExpr> expr) {
	textRepr << "IntToPtrCastExpr<";
	VISIT_THIS(expr->getType())
	textRepr << ">(";
	VISIT_THIS(expr->getOperand())
	textRepr << ")";
}

void ValueTextReprVisitor::visit(ShPtr<PtrToIntCastExpr> expr) {
	textRepr << "PtrToIntCastExpr<";
	VISIT_THIS(expr->getType())
	textRepr << ">(";
	VISIT_THIS(expr->getOperand())
	textRepr << ")";
}
// End of casts.

void ValueTextReprVisitor::visit(ShPtr<ConstBool> constant) {
	textRepr << (constant->getValue() ? "True" : "False");
}

void ValueTextReprVisitor::visit(ShPtr<ConstFloat> constant) {
	textRepr << constant->toString();
}

void ValueTextReprVisitor::visit(ShPtr<ConstInt> constant) {
	textRepr << constant->toString();
}

void ValueTextReprVisitor::visit(ShPtr<ConstNullPointer> constant) {
	textRepr << "NULL";
}

void ValueTextReprVisitor::visit(ShPtr<ConstString> constant) {
	textRepr << "\"";
	textRepr << constant->getValueAsEscapedCString();
	textRepr << "\"";
}

void ValueTextReprVisitor::visit(ShPtr<ConstArray> constant) {
	if (constant->isInitialized()) {
		textRepr << "[";
		bool first = true;
		for (const auto &element : constant->getInitializedValue()) {
			if (!first) {
				textRepr << ", ";
			}
			VISIT_THIS(element)
			first = false;
		}
		textRepr << "]";
	} else {
		// The array is not initialized.
		ArrayType::Dimensions dims(constant->getDimensions());
		if (dims.empty()) {
			textRepr << "[]";
			return;
		}

		// To prevent an emission of a lot of code, instead of emitting a
		// full-blown initializer, we emit just a call to array(). For example,
		// an initializer for an array of type `int [10][5][5]` is emitted as
		// `array(10, 5, 5)`.
		textRepr << "array(";
		bool first = true;
		for (const auto &dim : dims) {
			if (!first) {
				textRepr << ", ";
			}
			textRepr << dim;
			first = false;
		}
		textRepr << ")";
	}
}

void ValueTextReprVisitor::visit(ShPtr<ConstStruct> constant) {
	textRepr << "{";
	bool first = true;
	for (const auto &member : constant->getValue()) {
		if (!first) {
			textRepr << ", ";
		}
		textRepr << "'";
		VISIT_THIS(member.first)
		textRepr << "': ";
		VISIT_THIS(member.second)
		first = false;
	}
	textRepr << "}";
}

void ValueTextReprVisitor::visit(ShPtr<ConstSymbol> constant) {
	// (name -> value)
	textRepr << "(" << constant->getName() << " -> ";
	VISIT_THIS(constant->getValue())
	textRepr << ")";
}

void ValueTextReprVisitor::visit(ShPtr<AssignStmt> stmt) {
	VISIT_THIS(stmt->getLhs())
	textRepr << " = ";
	VISIT_THIS(stmt->getRhs())
}

void ValueTextReprVisitor::visit(ShPtr<VarDefStmt> stmt) {
	VISIT_THIS(stmt->getVar())

	if (ShPtr<Expression> init = stmt->getInitializer()) {
		textRepr << " = ";
		VISIT_THIS(init)
	}
}

void ValueTextReprVisitor::visit(ShPtr<CallStmt> stmt) {
	VISIT_THIS(stmt->getCall())
}

void ValueTextReprVisitor::visit(ShPtr<ReturnStmt> stmt) {
	textRepr << "return";
	if (ShPtr<Expression> retVal = stmt->getRetVal()) {
		textRepr << " ";
		VISIT_THIS(retVal)
	}
}

void ValueTextReprVisitor::visit(ShPtr<EmptyStmt> stmt) {
	textRepr << "# empty statement";
}

void ValueTextReprVisitor::visit(ShPtr<IfStmt> stmt) {
	// Emit the first if clause and other else-if clauses (if any).
	for (auto i = stmt->clause_begin(), e = stmt->clause_end(); i != e; ++i) {
		textRepr << (i == stmt->clause_begin() ? "if " : "\nelif ");
		VISIT_THIS(i->first)
		textRepr << ":";
	}

	// Emit the else clause (if any).
	if (stmt->hasElseClause()) {
		textRepr << "\nelse:";
	}
}

void ValueTextReprVisitor::visit(ShPtr<SwitchStmt> stmt) {
	textRepr << "switch ";
	VISIT_THIS(stmt->getControlExpr())
	textRepr << ":";
	for (auto i = stmt->clause_begin(), e = stmt->clause_end(); i != e; ++i) {
		textRepr << "\n";
		if (i->first) {
			textRepr << "case ";
			VISIT_THIS(i->first)
			textRepr<< ":";
		} else {
			textRepr << "default:";
		}
	}
}

void ValueTextReprVisitor::visit(ShPtr<WhileLoopStmt> stmt) {
	textRepr << "while ";
	VISIT_THIS(stmt->getCondition())
	textRepr << ":";
}

void ValueTextReprVisitor::visit(ShPtr<ForLoopStmt> stmt) {
	textRepr << "for ";
	VISIT_THIS(stmt->getIndVar())
	textRepr << " in range(";
	VISIT_THIS(stmt->getStartValue())
	textRepr << ", ";
	// If the end condition is of the form `i < x`, emit just `x`, otherwise
	// emit the complete condition.
	bool endCondEmitted = false;
	if (ShPtr<LtOpExpr> ltEndCond = cast<LtOpExpr>(stmt->getEndCond())) {
		if (stmt->getIndVar() == ltEndCond->getFirstOperand()) {
			VISIT_THIS(ltEndCond->getSecondOperand())
			endCondEmitted = true;
		}
	}
	if (!endCondEmitted) {
		stmt->getEndCond()->accept(this);
		VISIT_THIS(stmt->getEndCond())
	}
	// Emit step only if it differs from 1.
	ShPtr<ConstInt> stepInt = cast<ConstInt>(stmt->getStep());
	if (!stepInt || stepInt->getValue() != 1) {
		textRepr << ", ";
		stmt->getStep()->accept(this);
	}
	textRepr << "):";
}

void ValueTextReprVisitor::visit(ShPtr<UForLoopStmt> stmt) {
	// Use a C version because Python does not support our "universal" for
	// loops.
	textRepr << "for (";
	if (auto init = stmt->getInit()) {
		init->accept(this);
	}
	textRepr << "; ";
	if (auto cond = stmt->getCond()) {
		cond->accept(this);
	}
	textRepr << "; ";
	if (auto step = stmt->getStep()) {
		stmt->getStep()->accept(this);
	}
	textRepr << "):";
}

void ValueTextReprVisitor::visit(ShPtr<BreakStmt> stmt) {
	textRepr << "break";
}

void ValueTextReprVisitor::visit(ShPtr<ContinueStmt> stmt) {
	textRepr << "continue";
}

void ValueTextReprVisitor::visit(ShPtr<GotoStmt> stmt) {
	textRepr << "goto " << stmt->getTarget()->getLabel();
}

void ValueTextReprVisitor::visit(ShPtr<UnreachableStmt> stmt) {
	textRepr << "# UNREACHABLE";
}

void ValueTextReprVisitor::visit(ShPtr<FloatType> type) {
	textRepr << "double";
}

void ValueTextReprVisitor::visit(ShPtr<IntType> type) {
	if (type->isUnsigned()) {
		textRepr << "u";
	}
	textRepr << "int" << type->getSize();
}

void ValueTextReprVisitor::visit(ShPtr<PointerType> type) {
	textRepr << "ptr";
}

void ValueTextReprVisitor::visit(ShPtr<StringType> type) {
	textRepr << "string";
}

void ValueTextReprVisitor::visit(ShPtr<ArrayType> type) {
	textRepr << "array";
}

void ValueTextReprVisitor::visit(ShPtr<StructType> type) {
	textRepr << "struct (" <<
		(type->hasName() ? type->getName() : "anonymous") << ")";
}

void ValueTextReprVisitor::visit(ShPtr<FunctionType> type) {
	textRepr << "function";
}

void ValueTextReprVisitor::visit(ShPtr<VoidType> type) {
	textRepr << "void";
}

void ValueTextReprVisitor::visit(ShPtr<UnknownType> type) {
	textRepr << "unknown";
}

} // namespace llvmir2hll
} // namespace retdec
