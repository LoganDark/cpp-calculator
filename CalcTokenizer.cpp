#ifndef CALCULATOR_CALCTOKENIZER_CPP
#define CALCULATOR_CALCTOKENIZER_CPP

#include <string>
#include <vector>
#include <algorithm>
#include <iterator>

#include "CalcTokenizer.hpp"
#include "Tokenizer.cpp"
#include "CalcTkEnum.hpp"
#include "CalcASTException.hpp"
#include "CalcOperators.hpp"

template <class Num>
	std::wstring CalcTokenizer<Num>::var_chars = L"abcdefghijklmnopqrstuvwxyz"
	                                             L"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	                                             // greek alphabet
	                                             L"αβγδεζηθικλμνξοπρσςτυφχψω"
	                                             L"ΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡΣΤΥΦΧΨΩ"
	                                             L"_"; // lonely underscore

template <class Num>
	std::wstring CalcTokenizer<Num>::digits = L"0.123456789";

template <class Num>
	std::vector<CalcOperator<Num>> CalcTokenizer<Num>::binary_ops
		                               = CalcOperators<Num>::binary_ops;

template <class Num>
	std::vector<CalcUOperator<Num>> CalcTokenizer<Num>::unary_ops
		                                = CalcOperators<Num>::unary_ops;

template <class Num>
	std::wstring CalcTokenizer<Num>::parenthesis = L"()";

template <class Num>
template <class Op>
	bool CalcTokenizer<Num>::ops_start_with(std::vector<Op> & ops,
	                                        std::wstring prefix) {
		return std::find_if(ops.begin(), ops.end(), [&prefix](Op op) {
			return op.op.length() >= prefix.length() &&
			       op.op.substr(0, prefix.length()) == prefix;
		}) != ops.end();
	}

template <class Num>
template <class Op>
	bool CalcTokenizer<Num>::ops_start_with(std::vector<Op> & ops,
	                                        wchar_t prefix) {
		return std::find_if(ops.begin(), ops.end(), [&prefix](Op op) {
			return op.op[0] == prefix;
		}) != ops.end();
	}

/**
 * Consumes all characters from string `str`. See
 * [[CalcTokenizer<Num>::consume_push]] for a function that creates a new token.
 *
 * @param str The string to consume characters from.
 */
template <class Num>
	void CalcTokenizer<Num>::consume(const std::wstring & str) {
		while (STRING_CONTAINS(str, peek())) {
			move();
		}
	}

/**
 * Does all the main work, honestly. Uses the various [[Tokenizer]]
 * functions to chop up a string into little itty bitty bits and label them
 * based on an extremely strict set of rules.
 */
template <class Num>
	void CalcTokenizer<Num>::_tokenize() {
		while (true) {
			/**
			 * Remove whitespace before getting the token
			 */
			chomp_whitespace();

			wchar_t peeked = peek();

			if (peeked == -1) {
				/**
				 * If we are at the end of the input, exit
				 */
				break;
			} else if (STRING_CONTAINS(digits + var_chars, peeked)) {
				bool is_var = STRING_CONTAINS(var_chars, peeked);

				/**
				 * implied multiplication, e.g. 5x
				 */
				if (is_var && after_value()) {
					push_implied_token(TK_OPERATOR, L"*");
				}

				/**
				 * maybe check this _before_ we push a value to the stack
				 */
				bool throw_err = after_value();

				start_token();
				consume(is_var ? var_chars + digits : digits);
				push_token(is_var ? TK_VARIABLE : TK_NUMBER);

				/**
				 * Throw an error if the last token was a value
				 */
				if (throw_err) {
					throw CalcASTException(tokens.back(),
					                       L"Two values with no operation in"
					                       L" between");
				}

				/**
				 * There's some stuff going on here.
				 *
				 * First of all, the peeked character has to be at the start of
				 * an operator. Once that happens, one of the following
				 * conditions must be true:
				 *
				 * - The last token must not be an opening parenthesis (that
				 *   would imply a unary operator)
				 * - This operator cannot possibly be a unary operator
				 * - There is a space after this operator
				 *
				 * The next case handles unary operators where these conditions
				 * are not fulfilled.
				 */
			} else if (ops_start_with(binary_ops, peeked) && (
				!ops_start_with(unary_ops, peeked) || (
					tokens.empty() &&
					!immediately_before_value()
				) || !after_operator()
			)) {

				/**
				 * To be unary:
				 * - this has to be an actual unary operator
				 * - if it's not after a value, it must be after a unary
				 *   operator
				 */
				bool unary = ops_start_with(unary_ops, peeked) &&
				             (after_uop() || after_operator());

				/**
				 * Throw error if this is not unary but is after an operator
				 *
				 * @formatter:off
				 */
				bool throw_err = !unary && after_operator();
				// @formatter:on

				start_token();

				/**
				 * we know this is valid, we checked
				 */
				move();

				/**
				 * Consume all characters of the operator
				 */
				if (unary) {
					while (ops_start_with(unary_ops, get_token_data() +
					                                 peek())) {
						move();
					}
				} else {
					while (ops_start_with(binary_ops, get_token_data() +
					                                  peek())) {
						move();
					}
				}

				push_token(unary ? TK_UOPERATOR : TK_OPERATOR);

				/**
				 * This only happens if there are 2 operators in a row
				 */
				if (throw_err) {
					throw CalcASTException(tokens.back(),
					                       L"Too many operators");
				}
			} else if (ops_start_with(unary_ops, peeked)) {
				/**
				 * Anything that didn't pass as a binary operator but is in
				 * unary_ops should be counted as a unary operator, of course
				 *
				 * Not all unary operators are also binary operators
				 */
				start_token();

				/**
				 * Consume the first character, we know it's valid, we checked
				 */
				move();

				/**
				 * Consume all extra characters of the unary operator (for
				 * operators that span more than one character) until consuming
				 * any more characters would make it invalid
				 */
				while (ops_start_with(unary_ops, get_token_data() + peek())) {
					move();
				}

				bool after = uassociativity(get_token_data())
				             == UASSOCIATE_AFT;

				/**
				 * Throw an error if this is a trailing token after an operator,
				 * e.g. +! (factorial of addition operator)
				 *
				 * Also throws an error if we're after a leading unary operator,
				 * for example 2 + -! (2 plus negative factorial)
				 *
				 * Does not throw an error if this is the first token and it's
				 * a trailing unary operator, since implicit insertion would get
				 * it there. If not, AST will get it anyway
				 */
				bool throw_err = after ? !tokens.empty() && !after_value()
				                       : !after_operator();

				push_token(TK_UOPERATOR);

				if (throw_err) {
					throw CalcASTException(tokens.back(),
					                       after ? L"Missing operand for unary"
					                               L" operator"
					                             : L"Too many operators");
				}
			} else if (STRING_CONTAINS(parenthesis, peeked)) {
				bool  opening_paren       = peeked == parenthesis[0];
				Token matching_paren      =
					      get_matching_token(TK_OPAREN, TK_CPAREN, 1);
				Token matching_func_paren =
					      get_matching_token(TK_FOPAREN, TK_FCPAREN, 1);

				/**
				 * to be a function parenthesis, this needs to either:
				 * - be a closing parenthesis with a matching function opening
				 *   parenthesis
				 * or:
				 * - be an opening parenthesis coming immediately after a
				 *   variable name (in which case that variable name will be
				 *   upgraded to a function name)
				 */
				bool function_paren =
					     (
						     opening_paren && !tokens.empty() &&
						     tokens.back().type == TK_VARIABLE &&
						     STRING_CONTAINS(var_chars, peek(-1))
					     ) ||
					     (
						     !opening_paren &&
						     matching_func_paren.type == TK_FOPAREN &&
						     matching_func_paren.type != TK_UNKNOWN &&
						     (
							     matching_paren.type == TK_UNKNOWN ||
							     matching_func_paren.pos > matching_paren.pos
						     )
					     );

				if (opening_paren) {
					if (!function_paren && after_value()) {
						/**
						 * imply multiplication since 5(2) or something like that
						 *
						 * 5(2)
						 * (2 + 3)(2 + 3)
						 *
						 * etc...
						 */
						start_token();
						move();
						push_token(TK_OPERATOR);
						tokens.back().data = L"*";
						move(-1);
					} else if (function_paren) {
						tokens.back().type = TK_FUNCTION;
					}
				}

				start_token();
				std::wstring paren = get();

				/**
				 * if this is an opening parenthesis, push TK_OPAREN
				 *
				 * If it's not, it can only be one other type of parenthesis:
				 * TK_CPAREN
				 */
				push_token(function_paren
				           ? (opening_paren ? TK_FOPAREN : TK_FCPAREN)
				           : (opening_paren ? TK_OPAREN : TK_CPAREN));
			} else if (peeked == ',' &&
			           get_matching_token(TK_FOPAREN, TK_FCPAREN, 1).type ==
			           TK_FOPAREN) {
				start_token();
				move();
				push_token(TK_COMMA);
			} else {
				/**
				 * just grab the next character and push it, I guess?
				 */
				start_token();
				get();
				push_token(TK_UNKNOWN);

				/**
				 * throw an exception accordingly, the : thing can be confusing,
				 * I guess?
				 *
				 * There's unfortunately a difference between:
				 * one; :help
				 * one;:help
				 *
				 * as well as
				 * :help
				 *  :help
				 */
				if (peeked == ':') {
					throw CalcASTException(tokens.back(),
					                       L"Commands must start at the"
					                       L" beginning of a line");
				} else {
					throw CalcASTException(tokens.back(),
					                       L"Unknown character");
				}
			}
		}
	}

template <class Num>
	CalcTokenizer<Num>::CalcTokenizer(
		const std::wstring & str)
		: Tokenizer(str) {}

template <class Num>
	bool CalcTokenizer<Num>::after_value() {
		if (tokens.empty()) {
			return false;
		}

		Token & tk = tokens.back();

		return (tk.type & (TK_VARIABLE | TK_NUMBER | TK_CPAREN)) != 0 ||
		       (
			       tk.type == TK_UOPERATOR &&
			       uassociativity(tk.data) == UASSOCIATE_AFT
		       );
	}

template <class Num>
	bool CalcTokenizer<Num>::after_operator() {
		if (tokens.empty()) {
			return false;
		}

		return !after_value() &&
		       (tokens.back().type & (TK_OPERATOR | TK_OPAREN)) > 0;
	}

template <class Num>
	bool CalcTokenizer<Num>::after_uop() {
		if (tokens.empty()) {
			/**
			 * might as well return true if we're immediately before a number
			 * and the token stream is empty, the user clearly intends to imply
			 * a unary operator
			 *
			 * but if that's not the case, don't return true, if there's a space
			 * there's a likely chance the user wants an implied operation
			 * instead
			 */
			return immediately_before_value();
		}

		Token & tk = tokens.back();

		return tk.type == TK_UOPERATOR &&
		       uassociativity(tk.data) == UASSOCIATE_BEF;
	}

template <class Num>
	void CalcTokenizer<Num>::push_implied_token(TokenType type,
	                                            std::wstring op) {
		start_token();
		move();
		push_token(type);
		move(-1);
		tokens.back().data = op;
	}

template <class Num>
	UnaryAssociativity
	CalcTokenizer<Num>::uassociativity(std::wstring op) {
		return CalcOperators<Num>::unary_op_lookup.at(op).associativity;
	}

template <class Num>
	bool CalcTokenizer<Num>::immediately_before_value() {
		return STRING_CONTAINS(digits + var_chars + parenthesis[0],
		                       peek(1));
	}

template <class Num>
	Token CalcTokenizer<Num>::get_matching_token(TokenType opening,
	                                             TokenType closing,
	                                             int scope) {
		if (tokens.empty()) {
			return Token();
		}

		for (auto begin = tokens.begin(), current = --tokens.end();
		     current != begin; current--) {
			if (current->type == opening) {
				scope--;
			} else if (current->type == closing) {
				scope++;
			}

			if (scope == 0) {
				return * current;
			}
		}

		return Token();
	}

#endif //CALCULATOR_CALCTOKENIZER_CPP
