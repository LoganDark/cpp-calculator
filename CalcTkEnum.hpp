#ifndef CALCULATOR_CALCTKENUM_HPP
#define CALCULATOR_CALCTKENUM_HPP

#include <map>
#include <string>

#include "Token.hpp"

enum TokenType {
	/**
	 * An unknown value. Always instantly triggers a syntax error when
	 * encountered by the tokenizer.
	 */
	TK_UNKNOWN = 1,

	/**
	 * A number. This will cause an error if it's not an integer or decimal.
	 */
	TK_NUMBER = 2,

	/**
	 * An operator. These are used with postfix notation to run operations on
	 * numbers.
	 */
	TK_OPERATOR = 4,

	/**
	 * Unary operators! `+5` and `5!` are both examples of unary operators
	 */
	TK_UOPERATOR = 8,

	/**
	 * These are separate types because there can only be two values.
	 */
	TK_OPAREN = 16,
	TK_CPAREN = 32,

	/**
	 * Function equivalents of parenthesis (f((5 + 6), 3) has:
	 *             2 regular parenthesis      ^     ^
	 *             2 function parenthesis    ^          ^
	 *
	 * TK_FOPAREN always comes after a TK_FUNCTION token
	 */
	TK_FOPAREN = 64,
	TK_FCPAREN = 128,

	/**
	 * Variables! Can contain characters a-z, A-Z, and underscore
	 */
	TK_VARIABLE = 256,

	/**
	 * Function names, always preceding a TK_FOPAREN
	 */
	TK_FUNCTION = 512,

	/**
	 * Commas, separate arguments to functions
	 */
	TK_COMMA = 1024
};

/**
 * Token debug information.
 */
std::map<int, std::wstring> calc_debug_tk {
	{TK_UNKNOWN, L"UNKNOWN"},
	{TK_NUMBER, L"NUMBER"},
	{TK_OPERATOR, L"OPERATOR"},
	{TK_UOPERATOR, L"UOPERATOR"},
	{TK_OPAREN, L"OPAREN"},
	{TK_CPAREN, L"CPAREN"},
	{TK_FOPAREN, L"FOPAREN"},
	{TK_FCPAREN, L"FCPAREN"},
	{TK_VARIABLE, L"VARIABLE"},
	{TK_FUNCTION, L"FUNCTION"},
	{TK_COMMA, L"COMMA"}
};

#endif //CALCULATOR_CALCTKENUM_HPP
