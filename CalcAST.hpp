#ifndef CALCULATOR_CALCAST_HPP
#define CALCULATOR_CALCAST_HPP

#include <vector>
#include <map>

#include "Token.hpp"
#include "CalcASTElem.hpp"
#include "CalcASTEnum.hpp"
#include "CalcASTException.hpp"

/**
 * Namespace for AST generation
 */
namespace CalcAST {
	namespace {
		/**
		 * https://en.wikipedia.org/wiki/Shunting-yard_algorithm
		 *
		 * Wikipedia was very helpful in helping me implement this. It's not the
		 * most efficient version, but it works SO WELL.
		 *
		 * Most of this algorithm is explained there, but some modifications I
		 * made to it are explained via comments in the functions. I understand
		 * it isn't the most efficient implementation, but it works very well.
		 *
		 * @param tokens The tokens to, uh.. shunt?
		 * @return The same tokens, but in RPN order: (5 + 2) * 2 -> 5 2 + 2 *
		 * 5! * 2 = 5 ! 2 *
		 * -5 + 2 = 5 - 2 +
		 * Test examples can't show that the shunting yard knows the difference
		 * between unary and binary operators (the token types are different) as
		 * well as the AST.
		 */
		template <class Num>
			std::vector<Token> shunting_yard(const std::vector<Token> & tokens);
	}

	/**
	 * Get one expression from a RPN array. Imagine this:
	 *
	 * 5 3 + 2 *
	 *
	 * The function reads the multiplication sign and then calls itself for the
	 * right- and left-hand sides. The RHS finds 2 and immediately returns it as
	 * a [[CalcASTElem]]. The LHS finds an operator and creates a
	 * [[CalcASTElem]] representing 5 + 2. Recursion is the basic idea behind
	 * this. Then the function puts the LHS and RHS together into a
	 * [[CalcASTElem]] representing one operation and returns it.
	 *
	 * [[CalcAST::shunting_yard]] converts a flat (i.e. not tested) postfix
	 * expression into a flat RPN vector. Then this function converts that flat
	 * RPN vector into a nested structure that allows for easy execution.
	 *
	 * I could execute the RPN directly (i.e. skip the AST step), but where's
	 * the fun in that? This is also a validation step that prevents the input
	 * from being too garbled.
	 *
	 * @param rpn
	 * @return
	 */
	CalcASTElem get_exp_from_rpn(std::vector<Token> & rpn);

	/**
	 * This function simply calls [[CalcAST::get_exp_from_rpn]] and
	 * intentionally _copies_ the array you pass to it so
	 * [[CalcAST::get_exp_from_rpn]]'s side effects don't cause weirdness.
	 *
	 * @param rpn
	 * @return
	 */
	CalcASTElem rpn_to_ast(std::vector<Token> rpn);

	/**
	 * Generates an AST from tokens. Basically uses [[CalcAST::shunting_yard]]
	 * to generate an RPN token list and then uses [[CalcAST::rpn_to_ast]] to
	 * generate an AST out of that.
	 *
	 * @param tokens
	 * @return
	 */
	template <class Num>
		CalcASTElem generate_ast(const std::vector<Token> & tokens);
}

#endif //CALCULATOR_CALCAST_HPP
