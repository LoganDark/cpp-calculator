#ifndef CALCULATOR_CALCULATOR_CPP
#define CALCULATOR_CALCULATOR_CPP

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <sstream>
#include <iterator>
#include <iostream>

#include "Calculator.hpp"
#include "CalcTokenizer.cpp"
#include "CalcAST.cpp"
#include "CalcOperators.hpp"

#include "boilerplate/ld_boilerplate.hpp"

template <class Num>
	std::vector<Token> Calculator<Num>::tokenize(const std::wstring & input) {
		CalcTokenizer<Num> tokenizer(input);

		std::vector<Token> tokens = tokenizer.tokenize();

		if (tk_debug_force) {
			std::cout
				<< "FORCED tokens:\n" + LD::w2str(stringify_tks(tokens)) + "\n"
				<< std::endl;
		}

		return tokens;
	}

template <class Num>
	std::wstring Calculator<Num>::execute_command(std::vector<Token> args,
	                                              bool validate_only) {
		if (args.empty()) {
			throw CalcASTException(Token(), L"No command name");
		}

		std::wstring command_name = args.front().data.substr(1);

		try {
			return commands.at(command_name)(args, validate_only);
		} catch (std::out_of_range &) {
			throw CalcASTException(args[0], L"Unknown command");
		}
	}

template <class Num>
	CalcASTElem Calculator<Num>::get_ast(std::vector<Token> tokens) {
		Token & first = tokens.front();

		/**
		 * if the first token is an operator, the user may want to perform an
		 * operation on the last result
		 */
		if (!tokens.empty() && first.type & (TK_OPERATOR | TK_UOPERATOR)) {
			/**
			 * if the first token is indeed a regular, binary operator,
			 * then use the last result as the left hand side
			 *
			 * else, it must be a unary operator. if it is the only operator
			 * in the string, then implicitly perform the operator on the
			 * last result.
			 *
			 * else, if the next token isn't a number, variable, or opening
			 * parenthesis, insert the implicit last result anyway
			 */
			if (first.type == TK_OPERATOR || tokens.size() == 1 ||
			    !(
				    tokens.at(1)
				          .type & (TK_NUMBER | TK_VARIABLE | TK_OPAREN)
			    )) {
				try {
					variables.at(L"_");
					tokens.insert(tokens.begin(), Token(TK_VARIABLE, L"_", 0));
				} catch (std::out_of_range &) {
					throw CalcASTException(tokens.front(),
					                       L"No previous result for implicit"
					                       L" operation");
				}
			}
		}

		CalcASTElem ast = CalcAST::generate_ast<Num>(tokens, * this);

		if (ast_debug_force) {
			std::cout << "FORCED AST:\n" + LD::w2str(stringify_ast(ast)) + "\n"
			          << std::endl;
		}

		return ast;
	}

template <class Num>
	Calculator<Num>::Calculator() {
		commands[L"help"] =
			[this](const std::vector<Token> args,
			       bool validate_only) -> std::wstring {
				/**
				 * Help pages are only one argument long
				 */
				if (args.size() > 2) {
					throw CalcASTException(args[0],
					                       L"Expected 1-2 arguments, got " +
					                       LD::wtostring(args.size()));
				} else if (args.size() == 2) {
					Token help_page = args[1];

					try {
						/**
						 * This can throw an exception, so catch it
						 */
						return this->help_pages.at(help_page.data);
					} catch (std::out_of_range &) {
						throw CalcASTException(help_page,
						                       L"No such help page exists");
					}
				}

				/**
				 * If we get to here, there was no argument specified, so
				 * return an index of all help pages
				 */
				std::wstring help = L"There are multiple help pages. Run one"
				                    " of these commands to read them:\n";

				/**
				 * For every help page, add an entry to the string
				 */
				for (auto i = help_pages.begin();
				     i != help_pages.end(); i++) {
					help.append(L":help " + i->first + L"\n");
				}

				/**
				 * Remove the trailing newline
				 */
				return help + L"\nTry running :help starthere if you're"
				              L" new!";
			};
	};

template <class Num>
	std::vector<Token> Calculator<Num>::tokenize_command(
		const std::wstring & input) {
		/**
		 * Create a stream so we have a way of consuming words without weird
		 * iterators and index counters
		 */
		std::wstringstream cmd_stream(input);

		/**
		 * Variables
		 */
		std::wstring  word;
		std::wstring  trailing_white;
		unsigned long pos = 0;

		/**
		 * We will want arguments too
		 */
		std::vector<Token> words;

		/**
		 * Until otherwise stated, loop...
		 */
		while (true) {
			word.clear();
			trailing_white.clear();

			/**
			 * Get the next word in the string
			 */
			LD::consume_word(cmd_stream, word, & trailing_white);

			/**
			 * If there is no word to consume, there are no more arguments
			 * because we are at the end of the string
			 *
			 * but if there's trailing whitespace then there is possibly a word
			 * after this to consume
			 */
			if (word.empty()) {
				if (trailing_white.empty()) {
					break;
				} else {
					continue;
				}
			}

			/**
			 * ...else push this argument to the list and continue
			 */
			words.emplace_back(TK_UNKNOWN, word, pos);
			pos += trailing_white.length() + word.length();
		}

		return words;
	}

template <class Num>
	std::wstring Calculator<Num>::execute_command_str(
		const std::wstring & input, bool validate_only) {
		std::vector<Token> args = tokenize_command(input);

		/**
		 * Last, but not least, kick things off by passing all the info
		 * we've gathered to [[Calculator::execute_command]]
		 */
		return execute_command(args, validate_only);
	}

template <class Num>
	Token Calculator<Num>::join_tokens(const Token & tk1, const Token & tk2) {
		std::wstring  data = tk1.data;
		unsigned long pos  = tk1.pos;

		std::wstring  data2 = tk2.data;
		unsigned long pos2  = tk2.pos;

		/**
		 * if token 2 starts before token 1, we can't copy... we need to add
		 * space at the start of token 1 to hold it
		 */
		if (pos2 < pos) {
			/**
			 * calculate how much padding to add
			 */
			unsigned long delta = pos - pos2;

			/**
			 * make a new string with the padding we need
			 */
			std::wstring padding;
			padding.append(delta, ' ');
			padding.append(data);

			/**
			 * replace the data with the adjusted data and change the
			 * position to be accurate
			 */
			data = padding;
			pos  = pos2;
		}

		/**
		 * calculate how many characters to copy from the beginning
		 */
		unsigned long delta = pos2 - pos;

		/**
		 * calculate how long we want the data string based on that
		 */
		unsigned long desired_length = data2.length() + delta;

		/**
		 * if data is too short, pad with spaces
		 */
		if (data.length() < desired_length) {
			data.append(desired_length - data.length(), ' ');
		}

		/**
		 * create an iterator as the target to copy to, then set it to the
		 * correct position
		 */
		auto it = data.begin();
		std::advance(it, delta);

		/**
		 * copy the second token to the first
		 */
		std::copy(data2.begin(), data2.end(), it);

		return Token(tk2.type, data, pos);
	}

template <class Num>
	Token Calculator<Num>::get_token(const CalcASTElem & ast) {
		/**
		 * if it has no children, we hit the bottom, return so all recursive
		 * calls can finish
		 */
		if (ast.children.empty()) {
			return ast.token;
		}

		Token current_tk = ast.token;

		/**
		 * extend the current token using all children, calling this
		 * function recursively to extend all of those too and propagate up
		 * until all changes hit this one
		 */
		for (const CalcASTElem & elem : ast.children) {
			/**
			 * utilize recursion to account for all children
			 */
			current_tk = join_tokens(current_tk, get_token(elem));
		}

		/**
		 * picking any one token type would be inaccurate
		 */
		current_tk.type = TK_UNKNOWN;

		return current_tk;
	}

template <class Num>
	Num Calculator<Num>::execute_ast(const CalcASTElem & ast,
	                                 bool validate_only, bool set_) {
		if (set_) {
			return variables[L"_"] = execute_ast(ast, validate_only);
		}

		if (ast.token.type == TK_NUMBER) {
			/**
			 * This is as deep as we can go, numbers can't have children.
			 *
			 * Jokes aside, if this fails, we can't try anything else. Throw
			 * an exception and leave it up to the caller to resolve
			 * any catastrophic system failure.
			 */
			try {
				return Num(LD::w2str(ast.token.data));
			} catch (std::exception & e) {
				throw CalcASTException(ast.token, L"Invalid number (" +
				                                  LD::c2wstr(e.what()) +
				                                  L")");
			}
		} else if (ast.token.type == TK_VARIABLE) {
			/**
			 * Just like numbers, except this time we retrieve it from
			 * a map.
			 */
			try {
				return variables.at(ast.token.data);
			} catch (std::out_of_range &) {
				throw CalcASTException(ast.token, L"Variable doesn't exist");
			}
		} else if (ast.token.type & (TK_UOPERATOR | TK_OPERATOR)) {
			try {
				/**
				 * if it's a binary operator, great, if it's not, it must be
				 * unary (TK_UOPERATOR)
				 */
				return ast.token.type == TK_OPERATOR
				       ? CalcOperators<Num>::binary_op_lookup
					       .at(ast.token.data)
					       .func(this, ast, validate_only)
				       : CalcOperators<Num>::unary_op_lookup
					       .at(ast.token.data)
					       .func(this, ast, validate_only);
			} catch (std::out_of_range &) {
				throw CalcASTException(ast.token, L"Invalid operation");
			}
		} else if (ast.token.type == TK_FUNCTION) {
			bool regular_function  = true;
			bool variadic_function = true;

			CalcOpFunc(Num) * func = nullptr;

			try {
				func = & functions
					.at(ast.token.data)
					.at(static_cast<unsigned>(ast.children.size()));
			} catch (std::out_of_range &) {
				regular_function = false;
			}

			if (!regular_function) {
				try {
					func = & variadic_funcs
						.at(ast.token.data);
				} catch (std::out_of_range &) {
					variadic_function = false;
				}
			}

			if (regular_function || variadic_function) {
				return (* func)(this, ast, validate_only);
			} else {
				throw CalcASTException(get_token(ast),
				                       L"Invalid function call");
			}
		} else {
			throw CalcASTException(ast.token, L"Unknown AST element");
		}
	}

template <class Num>
	std::wstring Calculator<Num>::stringify_tk(const Token & tk) {
		return
			L"(" + LD::pad(calc_debug_tk.at(tk.type), 9) +
			L"@" + LD::pad(LD::s2wstr(std::to_string(tk.pos)), 4, ' ', true) +
			L") " + tk.data;
	}

template <class Num>
	std::wstring Calculator<Num>::stringify_tks(
		const std::vector<Token> & tokens) {
		std::wstring built;

		for (const Token & tk : tokens) {
			built.append(stringify_tk(tk) + L"\n");
		}

		return built.substr(0, built.length() - 1);
	}

template <class Num>
	std::wstring Calculator<Num>::stringify_ast(const CalcASTElem & ast,
	                                            const std::wstring & newline) {
		std::wstring built =
			             LD::pad(calc_debug_ast.at(ast.type), 10) +
			             L": " + stringify_tk(ast.token);

		unsigned long children = ast.children.size();

		/**
		 * This basically generates a tree structure.
		 */
		for (unsigned long i = 0; i < children; i++) {
			built.append(newline + (i == children - 1 ? L"└ " : L"├ ") +
			             stringify_ast(
				             ast.children[i],
				             newline + (i == children - 1 ? L"  " : L"| ")
			             ));
		}

		return built;
	}

template <class Num>
	std::wstring Calculator<Num>::execute(const std::wstring & input,
	                                      bool validate_only) {
		if (input.empty()) {
			return L"";
		}

		if (input[0] == ':') {
			return execute_command_str(input, validate_only);
		}

		std::vector<Token> tokens = tokenize(input);
		CalcASTElem        ast    = get_ast(tokens);

		/**
		 * 1. Set the result variable (_) to the result from executing the
		 *    string
		 * 2. Return the value of 1 (which is also the result of executing
		 *    the string)
		 */
		std::wstring result = to_string(execute_ast(ast, validate_only, true));

		/**
		 * If AST debug mode is enabled, print the AST
		 */
		if (ast_debug) {
			result = L"AST:\n" + stringify_ast(ast) + L"\n\n" + result;
		}

		/**
		 * If TK debug mode is enabled, print the token list
		 */
		if (tk_debug) {
			result = L"Tokens:\n" + stringify_tks(tokens) + L"\n\n" + result;
		}

		/**
		 * Finally, return the result.
		 */
		return result;
	}

template <class Num>
	std::vector<std::wstring> Calculator<Num>::execute_multiple(
		const std::wstring & input, bool validate_only) {
		std::vector<std::wstring> commands;
		unsigned long             i    = 0;
		unsigned long             last = 0;

		for (const wchar_t & character : input) {
			if (character == ';') {
				/**
				 * Store this segment.
				 *
				 * one; two; three
				 *    ^      `last`
				 *         ^ `i`
				 *     ^^^^  `last` to `i`
				 */
				commands.push_back(input.substr(last, i - last));
				last = i + 1;
			}

			i++;
		}

		/**
		 * Store the last segment.
		 *
		 * one; two; three
		 *         ^        `last`
		 *                ^ `i`
		 *          ^^^^^^  after `last`
		 */
		commands.push_back(input.substr(last, i - last));

		/**
		 * Stores all the return values created by the various executions
		 * that will be performed
		 */
		std::vector<std::wstring> return_values;

		/**
		 * Executes all our commands and stores the results in
		 * `return_values`
		 */
		for (const std::wstring & command : commands) {
			return_values.push_back(execute(command, validate_only));
		}

		return return_values;
	}

#endif //CALCULATOR_CALCULATOR_CPP
