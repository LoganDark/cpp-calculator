#ifndef CALCULATOR_CALCAST_CPP
#define CALCULATOR_CALCAST_CPP

#include <vector>
#include <map>

#include "CalcAST.hpp"
#include "Token.hpp"
#include "CalcASTElem.hpp"
#include "CalcASTEnum.hpp"
#include "CalcASTException.hpp"
#include "CalcOperators.hpp"

#include "boilerplate/ld_boilerplate.hpp"

/**
 * Namespace for AST generation
 */
namespace CalcAST {
	namespace {
		template <class Num>
			std::vector<Token> shunting_yard(
				const std::vector<Token> & tokens) {
				auto & bin_lookup = CalcOperators<Num>::binary_op_lookup;
				auto & un_lookup  = CalcOperators<Num>::unary_op_lookup;

				/**
				 * The output stack - basically the RPN this function creates
				 */
				std::vector<Token> output;

				/**
				 * Pending operators, including parenthesis. These operators
				 * need some numbers to make them valid.
				 */
				std::vector<Token> op_stack;

				/**
				 * Function names. These are pushed onto the stack when a
				 * FCPAREN (closing function parenthesis) is encountered.
				 */
				std::vector<Token> func_stack;

				/**
				 * Simply here to keep track of when a [[TK_FCPAREN]]
				 * is encountered so we can pass it to the caller.
				 */
				Token to_push_last;

				for (auto end = tokens.end(), current = tokens.begin();
				     current != end; current++) {
					Token tk = * current;

					if (tk.type == TK_NUMBER || tk.type == TK_VARIABLE) {
						output.push_back(tk);

						if (!op_stack.empty() &&
						    op_stack.back().type == TK_UOPERATOR) {
							output.push_back(op_stack.back());
							op_stack.pop_back();
						}
					} else if (tk.type == TK_OPERATOR) {
						while ((
							!op_stack.empty() &&
							(
								op_stack.back().type != TK_OPAREN && (
									bin_lookup.at(op_stack.back().data).order >
									bin_lookup.at(tk.data).order ||
									(
										bin_lookup.at(op_stack.back().data)
										          .order ==
										bin_lookup.at(tk.data).order &&
										bin_lookup.at(op_stack.back().data)
										          .associativity == ASSOCIATE_L
									)
								)
							)
						)) {
							output.push_back(op_stack.back());
							op_stack.pop_back();
						}

						op_stack.push_back(tk);
					} else if (tk.type == TK_UOPERATOR) {
						/**
						 * if the operator is expected to come after the number
						 */
						if (un_lookup.at(tk.data).associativity
						    == UASSOCIATE_AFT) {
							output.push_back(tk);
						} else {
							/**
							 * else when a number is encountered, this unary op
							 * will be popped back off
							 */
							op_stack.push_back(tk);
						}
					} else if (tk.type == TK_OPAREN) {
						op_stack.push_back(tk);
					} else if (tk.type == TK_CPAREN) {
						while (!op_stack.empty() &&
						       op_stack.back().type != TK_OPAREN) {
							output.push_back(op_stack.back());
							op_stack.pop_back();
						}

						/**
						 * no opening parenthesis could be found
						 */
						if (op_stack.empty()) {
							throw CalcASTException(tk, L"Extra closing"
							                           L" parenthesis");
						}

						op_stack.pop_back();
					} else if (tk.type == TK_FUNCTION) {
						func_stack.push_back(tk);
					} else if (tk.type == TK_FCPAREN || tk.type == TK_COMMA) {
						/**
						 * End this - we can't possibly come upon this unless
						 * this is recursion, in which case, break to the caller
						 */
						to_push_last = tk;

						break;
					} else if (tk.type == TK_FOPAREN) {
						unsigned arity         = 0;
						Token    function      = func_stack.back();
						auto     opening_paren = * current;

						while (current->type != TK_FCPAREN) {
							/**
							 * Pass the rest of the tokens to a recursive
							 * function
							 */
							std::vector<Token> sub(++current, tokens.end());
							std::vector<Token> subrpn =
								                   shunting_yard<Num>(sub);

							if (subrpn.empty()) {
								throw CalcASTException(opening_paren,
								                       L"Unmatched parenthesis");
							}

							if (subrpn.back().type != TK_FCPAREN &&
							    subrpn.back().type != TK_COMMA) {
								/**
								 * the recursive call didn't find a closing
								 * function parenthesis or comma, unmatched
								 * parenthesis
								 */

								throw CalcASTException(opening_paren,
								                       L"Unmatched parenthesis");
							} else if (subrpn.size() == 1) {
								/**
								 * handles no arguments, grumble grumble logic
								 * sucks grumble grumble
								 */
								break;
							} else {
								/**
								 * the token the recursion stopped at
								 */
								Token & goal = subrpn.back();

								for (Token subtk : subrpn) {
									if (subtk.type != goal.type) {
										output.push_back(subtk);
									}
								}

								/**
								 * find the goal token
								 */
								for (auto subcurrent = current;
								     subcurrent != end; subcurrent++) {
									if (subcurrent->type == goal.type &&
									    subcurrent->pos == goal.pos) {
										/**
										 * we've found it, skip there
										 * the for loop will increment this if
										 * the while loop breaks because of it
										 */
										current = subcurrent;

										break;
									}
								}
							}

							arity++;
						}

						output.emplace_back(TK_NUMBER, LD::wtostring(arity),
						                    function.pos);
						output.push_back(function);

						func_stack.pop_back();
					}
				}

				while (!op_stack.empty()) {
					if (op_stack.back().type == TK_OPAREN) {
						/**
						 * no closing parenthesis popped this opening
						 * parenthesis off of the stack, so there are unmatched
						 * parenthesis
						 */
						throw CalcASTException(op_stack.back(),
						                       L"Extra opening parenthesis");
					}

					output.push_back(op_stack.back());
					op_stack.pop_back();
				}

				if (to_push_last.type != TK_UNKNOWN) {
					output.push_back(to_push_last);
				}

				return output;
			}
	}

	template <class Num>
		CalcASTElem get_exp_from_rpn(std::vector<Token> & rpn,
		                             Calculator<Num> & calc) {
			Token last = rpn.back();
			rpn.pop_back();

			if (last.type == TK_UOPERATOR) {
				if (rpn.empty()) {
					throw CalcASTException(last,
					                       L"Missing operand for unary"
					                       " operator");
				}

				return CalcASTElem(AST_UOPERATION, last,
				                   {get_exp_from_rpn(rpn, calc)});
			} else if (last.type & (TK_NUMBER | TK_VARIABLE)) {
				if (last.type == TK_NUMBER) {
					try {
						Num(LD::w2str(last.data));
					} catch (std::exception & exc) {
						throw CalcASTException(last, L"Invalid number (" +
						                             LD::c2wstr(exc.what()) +
						                             L")");
					}
				}

				return CalcASTElem(AST_VALUE, last);
			} else if (last.type == TK_OPERATOR) {
				/**
				 * last.type == TK_OPERATOR, since the last `if` statement didn't return
				 *
				 * We are parsing an operator, there must be 2 operands.
				 */
				if (rpn.empty()) {
					throw CalcASTException(last,
					                       L"Missing operand for binary operator");
				}

				/**
				 * Get the right- and left-hand side. I like the AST to resemble
				 * the input to aid debugging, so we get the right- and left-
				 * hand sides in this order to preserve the original order.
				 *
				 * Plus operations also expect the LHS and RHS to be in order.
				 */
				CalcASTElem right = get_exp_from_rpn(rpn, calc);

				/**
				 * Do this again because issues can crop up between `right` and
				 * `left` if `right` consumes more than 1 token
				 */
				if (rpn.empty()) {
					Token & to_throw = last;

					/**
					 * if `right.token` happens later than `last` in the order
					 * of operations, make this error correct
					 */
					if (right.token.pos > to_throw.pos &&
					    right.token.type != TK_UOPERATOR) {
						to_throw = right.token;
					}

					throw CalcASTException(to_throw,
					                       L"Missing operand for binary"
					                       L" operator");
				}

				CalcASTElem left = get_exp_from_rpn(rpn, calc);

				return CalcASTElem(AST_OPERATION, last, {left, right});
			} else if (last.type == TK_FUNCTION) {
				if (rpn.empty()) {
					throw CalcASTException(last, L"Function arity not found");
				}

				/**
				 * arity is the number of arguments a function requires
				 */
				auto arity = LD::from_string<unsigned>(rpn.back().data);
				rpn.pop_back();

				CalcASTElem func_elem = CalcASTElem(AST_CALL, last);

				for (unsigned i = 0; i < arity; i++) {
					if (rpn.empty()) {
						/**
						 * why would this happen? the shunting yard
						 * automatically assigned an arity based on how many
						 * arguments there were!!
						 *
						 * this is an edge case that, in theory, would never
						 * happen, but just in case it does, complain loudly,
						 * since I'm not perfect and I could've made a mistake
						 */
						throw CalcASTException(last, L"Not enough arguments?");
					}

					/**
					 * insert at the beginning because the shunting yard inserts
					 * them in the RPN in order... which is a problem because
					 * RPN is reversed, we're working from the end here
					 *
					 * so solve this by inserting at the beginning for every
					 * element
					 */
					func_elem.children.insert(func_elem.children.begin(),
					                          get_exp_from_rpn(rpn, calc));
				}

				return func_elem;
			} else {
				throw CalcASTException(last, L"Unknown token");
			}
		}

	template <class Num>
		CalcASTElem rpn_to_ast(std::vector<Token> rpn, Calculator<Num> & calc) {
			/**
			 * Nothing to see here, move along
			 */
			if (rpn.empty()) {
				throw std::runtime_error(
					"What is even going on here? What did you even put into"
					" this poor calculator? You monster."
				);
			}

			/**
			 * This function's purpose is purely to retrieve one expression from the
			 * RPN. In this case, EVERY valid expression has one last operation that
			 * will be performed, as well as many (or no) child operations that will
			 * be performed within it.
			 *
			 * For example: x 2 5 * 7 + =
			 *     which is x = 2 * 5 + 7
			 *
			 * Get one expression: [[CalcAST::get_exp_from_rpn]] sees `=`, and
			 * gets two more expressions, but one the first, recursion sees `+`,
			 * gets two more, sees `*`, gets two more. `*` returns, `+` returns,
			 * `=` gets one more value which is `x` and then returns as well.
			 *
			 * Result: =
			 *         ├ x
			 *         └ +
			 *           ├ *
			 *           │ ├ 2
			 *           │ └ 5
			 *           └ 7
			 */
			CalcASTElem exp = get_exp_from_rpn(rpn, calc);

			/**
			 * This error expresses my confusion perfectly
			 */
			if (!rpn.empty()) {
				throw CalcASTException(rpn.back(), L"RPN is not empty");
			}

			return exp;
		}

	/**
	 * Generates an AST from tokens. Basically uses [[CalcAST::shunting_yard]]
	 * to generate an RPN token list and then uses [[CalcAST::rpn_to_ast]] to
	 * generate an AST out of that.
	 *
	 * @param tokens
	 * @return
	 */
	template <class Num>
		CalcASTElem generate_ast(const std::vector<Token> & tokens,
		                         Calculator<Num> & calc) {
			/**
			 * Get RPN, which [[CalcAST::rpn_to_ast]] parses, from the list of
			 * tokens
			 */
			std::vector<Token> rpn = shunting_yard<Num>(tokens);

			try {
				/**
				 * Return the AST equivalent of said RPN
				 */
				return rpn_to_ast(rpn, calc);
			} catch (std::runtime_error &) {
				Token tk = tokens.empty() ? Token() : tokens.back();

				throw CalcASTException(tk, L"Generates empty AST");
			}
		}
}

#endif //CALCULATOR_CALCAST_CPP
