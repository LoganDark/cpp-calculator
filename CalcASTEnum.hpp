#ifndef CALCULATOR_CALCASTENUM_HPP
#define CALCULATOR_CALCASTENUM_HPP

#include <map>
#include <string>

enum {
	/**
	 * A value, like a number or variable. The token type specifies which one.
	 *
	 * Must have zero children.
	 */
	AST_VALUE,

	/**
	 * An operation. Must have two children, the left hand side and right hand
	 * side, in order. The token specifies which operation it is.
	 */
	AST_OPERATION,

	/**
	 * A unary operation. Must have only one child. The token specifies which
	 * operation it is.
	 */
	AST_UOPERATION,

	/**
	 * A call to a function. Can have a variable amount of children.
	 */
	AST_CALL
};

/**
 * AST debug information.
 */
std::map<int, std::wstring> calc_debug_ast {
	{AST_VALUE, L"VALUE"},
	{AST_OPERATION, L"OPERATION"},
	{AST_UOPERATION, L"UOPERATION"}
};

#endif //CALCULATOR_CALCASTENUM_HPP
