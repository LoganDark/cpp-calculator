#ifndef CALCULATOR_TOKEN_HPP
#define CALCULATOR_TOKEN_HPP

#include <string>
#include "CalcTkEnum.hpp"

/**
 * Represents a single token. This is often included in a linear list and these
 * cannot be nested like a [[CalcASTElem]] can.
 */
struct Token {
	/**
	 * The type of the token. See `CalcTkEnum.hpp` for the types this program
	 * uses.
	 */
	int type;

	/**
	 * The content of a token. Variable name for variables, number for numbers,
	 * an operator like "=" or "*" for operators. Normal token-like behavior.
	 */
	std::wstring data;

	/**
	 * The position of the token in the input string it came from.
	 */
	unsigned long pos;

	/**
	 * Create a new token.
	 *
	 * @param type The type of the new token.
	 * @param data The data of the new token.
	 * @param pos The position of the new token.
	 */
	Token(int type, std::wstring data, unsigned long pos)
		: type(type), data(std::move(data)), pos(pos) {}

	Token() : type(TK_UNKNOWN), data(L""), pos(0) {}
};

#endif //CALCULATOR_TOKEN_HPP
