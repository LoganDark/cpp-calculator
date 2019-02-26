#include <utility>

#ifndef CALCULATOR_CALCASTELEM_HPP
#define CALCULATOR_CALCASTELEM_HPP

#include <vector>

#include "Token.hpp"

/**
 * Represents an element of an AST. Has a token and a type that explain what it
 * does, and can have any number of children.
 */
struct CalcASTElem {
	/**
	 * The type. See `CalcASTEnum.hpp` for the different types this program
	 * uses.
	 */
	int type;

	/**
	 * The token this AST element represents. Usually this is the operation
	 * being performed, or in the case of values, the actual value.
	 */
	Token token;

	/**
	 * Values usually don't have these, but operators do. For example, binary
	 * operators always have 2 children and unary operators always have one.
	 *
	 * This allows for the nested structure that ASTs are known for having.
	 */
	std::vector<CalcASTElem> children;

	/**
	 * Create a new [[CalcASTElem]].
	 *
	 * @param type The type to give it.
	 * @param token The token it represents.
	 * @param children The children. Defaults to an empty vector.
	 */
	CalcASTElem(int type, Token token, std::vector<CalcASTElem> children = {})
		: type(type), token(std::move(token)), children(std::move(children)) {};
};

#endif //CALCULATOR_CALCASTELEM_HPP
