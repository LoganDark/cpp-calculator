#ifndef CALCULATOR_RATIONALCALCULATOR_CPP
#define CALCULATOR_RATIONALCALCULATOR_CPP

#include <vector>
#include <string>
#include <valarray>

#include "RationalCalculator.hpp"
#include "Calculator.cpp"
#include "boilerplate/precision/math_Rational.h"
#include "Irrational.hpp"

bool RationalCalculator::is_int(const math::Rational & num) {
	return num.denominator().abs() == 1;
}

std::wstring RationalCalculator::commatize_str(const std::wstring & str) {
	if (str.length() <= 3) {
		return str;
	}

	unsigned long rem   = str.length() % 3;
	std::wstring  built = str.substr(0, rem);

	for (unsigned long i = rem; i < str.length(); i += 3) {
		built += L"," + str.substr(i, 3);
	}

	return built.substr(static_cast<unsigned long>(rem == 0));
}

std::wstring RationalCalculator::to_string(const math::Rational & num) {
	if (precision == -2) {
		if (num.denominator() == 1) {
			return commatize_str(LD::s2wstr(num.to_string()));
		}

		return commatize_str(LD::s2wstr(num.numerator().to_string())) + L"/" +
		       commatize_str(LD::s2wstr(num.denominator().to_string()));
	}

	std::wstring decimal;

	if (precision < 0) {
		decimal = LD::s2wstr(num.to_precise_string());
	} else {
		decimal = LD::s2wstr(num.to_string(static_cast<size_t>(precision)));
	}

	if (commatize) {
		unsigned long after_sign = decimal[0] == '-' ? 1 : 0;
		unsigned long radix_pos  = decimal.find_first_of('.');

		/**
		 * there is only an integer portion (and possibly sign) - no decimal
		 */
		if (radix_pos == std::wstring::npos) {
			/**
			 * Sign + commatized integer portion
			 */
			return decimal.substr(0, after_sign) +
			       commatize_str(decimal.substr(after_sign));
		}

		/**
		 * The sign + the integer part + the radix point + the decimal part
		 */
		return decimal.substr(0, after_sign) +
		       commatize_str(decimal.substr(after_sign, radix_pos -
		                                                after_sign)) +
		       L"." + decimal.substr(radix_pos + 1);
	}

	return decimal;
}

RationalCalculator::RationalCalculator() : Calculator<math::Rational>() {
	register_commands();
	generate_commands_help();
	register_functions();
	generate_functions_help();
}

void RationalCalculator::register_commands() {
	/**
	 * document function added manually in main.cpp
	 */
	help_pages[L":exit"] = L":exit exits the calculator. That's it, really.\n"
	                       L"Usage: :exit";

	help_pages[L":prec"] =
		L":prec - set the precision of displayed numbers.\n"
		L"Usage: :prec <#/off>\n"
		L"Example: :prec 20 - displays 20 digits of precision\n"
		L"Example: :prec off - displays results to 'infinite' precision -"
		L" detects repeating decimals. For examplen 1/6 == 0.1(6)\n\n"
		L"Example: :prec frac - displays results using fractions (e.g. 1/3)\n\n"
		L"This only affects the output. Only the :irrational commands require"
		L" this to be some number of digits.\n"
		L"# must be a positive integer. It may be zero.";

	commands[L"prec"] =
		[this](std::vector<Token> args,
		       bool validate_only = false) -> std::wstring {
			if (args.size() == 2) {
				Token proposed = args[1];

				/**
				 * Throw an exception early for validation purposes
				 */
				if (proposed.data != L"off" && proposed.data != L"frac" &&
				    proposed.data.find_first_not_of(L"0123456789") !=
				    std::wstring::npos) {
					throw CalcASTException(proposed,
					                       L"Not a positive integer, `off` or"
					                       L" `frac`");
				}

				/**
				 * Only actually execute if we're not only validating
				 */
				if (!validate_only) {
					/**
					 * Set precision to a negative number to signal we want
					 * fractions instead
					 */
					if (proposed.data == L"off") {
						precision = -1;
					} else if (proposed.data == L"frac") {
						precision = -2;
					} else {
						/**
						 * Try to shove the input into `precision`. If it fails,
						 * complain loudly
						 */
						try {
							std::wstringstream ss(proposed.data);
							ss >> precision;
						} catch (std::exception &) {
							throw CalcASTException(proposed,
							                       L"Can't convert to integer");
						}
					}
				}

				return L"Precision set!";
			}

			throw CalcASTException(args[0], L"Expected 1 argument, got " +
			                                LD::wtostring(args.size() - 1));
		};

	help_pages[L":debug"] =
		L"Various debug utilities for looking at the calculator's internal,"
		L" intermediate states before a result has finished being"
		L" evaluated.\n"
		L"FORCING something causes it to be printed as soon as it's"
		L" available. Even if the command didn't finish completely, even if"
		L" it fails halfway through, or even if the result is thrown away,"
		L" debug information will still be printed.\n\n"
		L"Usage: :debug [tokens/ast] <on/off/force>\n"
		L"Example: :debug ast on - turns on AST printing\n"
		L"Example: :debug ast force - FORCES AST printing\n"
		L"Example: :debug on - turns on ALL printing (works with 'off' and"
		L" 'force' too)";

	commands[L"debug"] =
		[this](const std::vector<Token> args,
		       bool validate_only = false) -> std::wstring {
			/**
			 * if there are 2 arguments, the user specified either tokens or ast
			 * the command name is included in args
			 */
			if (args.size() == 2 || args.size() == 3) {
				Token group   = args[0],
				      setting = args[1];

				if (args.size() == 3) {
					group   = args[1];
					setting = args[2];
				}

				/**
				 * Validate early
				 */
				if (args.size() == 3 && group.data != L"tokens" &&
				    group.data != L"ast") {
					throw CalcASTException(group,
					                       L"Must be either `tokens` or"
					                       L" `ast`");
				}

				if (setting.data != L"on" && setting.data != L"off" &&
				    setting.data != L"force") {
					throw CalcASTException(setting, L"Setting must be `on`,"
					                                L" `off`, or `force`");
				}

				if (!validate_only) {
					bool set_on = setting.data == L"on",
					     force  = setting.data == L"force";

					if (args.size() == 2) {
						tk_debug_force  = force;
						tk_debug        = set_on;
						ast_debug_force = force;
						ast_debug       = set_on;
					} else {
						bool is_tokens = group.data == L"tokens",
							* target       = & (
							is_tokens ? tk_debug : ast_debug
						),
							* force_target = & (
								is_tokens ? tk_debug_force : ast_debug_force
							);

						/**
						 * `set_on` doesn't matter if `force` is `true`
						 */
						* force_target = force;
						* target       = set_on;
					}
				}

				return L"Debug state successfully set!";
			}

			throw CalcASTException(args[0],
			                       L"Expected 1 or 2 argument, got " +
			                       LD::wtostring(args.size() - 1));
		};

	help_pages[L":vars"] = L":vars prints all variables you have set.\n"
	                       L"Usage: :vars";

	commands[L"vars"] =
		[this](const std::vector<Token> & args,
		       bool validate_only = false) -> std::wstring {
			if (args.size() != 1) {
				throw CalcASTException(args[0],
				                       L"Expected no arguments, got " +
				                       LD::wtostring(args.size() - 1));
			}

			std::wstring built;

			/**
			 * display _, or display that there is no last result
			 */
			try {
				built = L"_: " + to_string(variables.at(L"_"));
			} catch (std::out_of_range &) {
				built = L"_: no last result";
			}

			/**
			 * add every variable except for _ (already accounted for) to the
			 * string, using built-in to_string for each one
			 */
			for (auto & variable : variables) {
				if (variable.first != L"_") {
					built.append(L"\n" + variable.first +
					             L": " + to_string(variable.second));
				}
			}

			return built;
		};

	help_pages[L":clear"] = L":clear resets the calculator, removing all"
	                        L" variables.\n"
	                        L"Usage: :clear";

	commands[L"clear"] =
		[this](const std::vector<Token> & args,
		       bool validate_only = false) -> std::wstring {
			if (args.size() != 1) {
				throw CalcASTException(args[0],
				                       L"Expected no arguments, got " +
				                       LD::wtostring(args.size() - 1));
			}

			if (!validate_only) {
				variables.clear();
			}

			return L"Variables successfully cleared!";
		};

	help_pages[L":delvar"] = L":delvar deletes a variable.\n"
	                         L"Usage: :delvar <variable>\n"
	                         L"Example: :delvar x - removes the variable `x`";

	commands[L"delvar"] =
		[this](const std::vector<Token> & args,
		       bool validate_only = false) -> std::wstring {
			if (args.size() != 2) {
				throw CalcASTException(args[0],
				                       L"Expected 1 argument, got " +
				                       LD::wtostring(args.size() - 1));
			}

			Token to_remove = args[1];

			try {
				variables.at(to_remove.data);

				if (!validate_only) {
					variables.erase(to_remove.data);
				}

				return L"Variable successfully removed!";
			} catch (std::out_of_range &) {
				throw CalcASTException(to_remove,
				                       L"No such variable exists");
			}
		};

	help_pages[L":commas"] = L":commas sets whether or not to show numbers with"
	                         L" thousands separators.\n"
	                         L"Usage: :commas <on/off>\n"
	                         L"Example: :commas off - hides thousands"
	                         L" separators";

	commands[L"commas"] =
		[this](const std::vector<Token> & args,
		       bool validate_only = false) -> std::wstring {
			/**
			 * if there isn't exactly 1 argument or it isn't on/off, display
			 * help
			 */
			if (args.size() != 2) {
				throw CalcASTException(args[0],
				                       L"Expected 1 argument, got " +
				                       LD::wtostring(args.size() - 1));
			}

			Token setting = args[1];

			if (setting.data != L"on" && setting.data != L"off") {
				throw CalcASTException(setting, L"Must be `on` or `off`");
			}

			if (!validate_only) {
				commatize = setting.data == L"on";
			}

			return L"Thousands separators turned " + setting.data + L"!";
		};

	help_pages[L":irrational"] =
		L"Various utilities related to irrational numbers.\n\n"
		L"Usage: :irrational <pi/e/golden> <var> - puts pi, e, or the golden"
		L" ratio into `var`. Uses the current precision as set by :prec.";

	commands[L"irrational"] =
		[this](const std::vector<Token> & args, bool validate_only = false)
			-> std::wstring {
			if (args.size() != 3) {
				throw CalcASTException(args[0],
				                       L"Expected 2 arguments, got " +
				                       LD::wtostring(args.size() - 1));
			}

			Token subcommand = args[1];

			if (subcommand.data != L"pi" && subcommand.data != L"e" &&
			    subcommand.data != L"golden") {
				throw CalcASTException(subcommand,
				                       L"Invalid subcommand (pi/e/golden)");
			}

			Token variable = args[2];

			if (!CONFORMS(variable.data,
			              CalcTokenizer<math::Rational>::var_chars)) {
				throw CalcASTException(variable, L"Invalid variable name");
			}

			if (precision < 0) {
				throw CalcASTException(args[0],
				                       L"Can't calculate perfectly (precision"
				                       L" must be set, see :help :prec)");
			}

			if (!validate_only) {
				try {
					if (subcommand.data == L"pi") {
						variables[variable.data] = Irrational::pi(precision);
					} else if (subcommand.data == L"e") {
						variables[variable.data] = Irrational::e(
							static_cast<size_t>(precision));
					} else if (subcommand.data == L"golden") {
						variables[variable.data] = Irrational::golden_ratio(
							static_cast<size_t>(precision));
					} else if (subcommand.data == L"sqrt") {
						variables[variable.data] = Irrational::sqrt(
							static_cast<size_t>(precision), variables.at(L"_"));
					}
				} catch (std::exception & e) {
					throw CalcASTException(args[1],
					                       LD::s2wstr(std::string(e.what())));
				}
			}

			return L"Success!";
		};

	help_pages[L":sort"] =
		L":sort sorts a sequence of numbers in ascending order.\n"
		L"Example: :sort 3 8 1 6 -> 1, 3, 6, 8";

	commands[L"sort"] =
		[this](const std::vector<Token> & args, bool validate_only = false)
			-> std::wstring {
			std::vector<math::Rational> sorted;
			std::vector<Token>          args2 = args;

			args2.erase(args2.begin());

			for (const Token & tk : args2) {
				try {
					sorted.emplace_back(LD::w2str(tk.data));
				} catch (std::exception & e) {
					throw CalcASTException(tk,
					                       L"Unable to convert to number: " +
					                       LD::s2wstr(e.what()));
				}
			}

			std::sort(sorted.begin(), sorted.end());

			std::wstring built;

			for (math::Rational & num : sorted) {
				//built.append(to_string(num) + L", ");
				built.append(LD::s2wstr(num.to_string()) + L", ");
			}

			return built.substr(0, built.length() - 2);
		};
}

void RationalCalculator::generate_commands_help() {
	std::wstring cmds_help =
		             L"This calculator has a few extra commands (besides :help)"
		             L" that can either affect how the calculator behaves or"
		             L" provide convenience. Run :help :<command> to see"
		             L" command information. For example, :help :prec will show"
		             L" help on the :prec command. Here's a list of"
		             L" commands:\n\n";

	for (auto & page : help_pages) {
		if (page.first[0] == ':') {
			cmds_help.append(page.first + L", ");
		}
	}

	help_pages[L"commands"] = cmds_help.substr(0, cmds_help.length() - 2);
	help_pages.at(L"starthere").append(L"\n:help commands");
}

void RationalCalculator::register_functions() {
	help_pages[L"sqrt()"] =
		L"sqrt() calculates the square root of a value.\n"
		L"Usage: sqrt(<value>)\n"
		L"Example: sqrt(4) - calculates the square root of 4. Returns 2.\n"
		L"Example: sqrt(2) - calculates the square root of 2. Returns"
		L" approximately 1.4142135623730950488.";

	functions[L"sqrt"].insert(
		// @formatter:off
		std::make_pair<unsigned, CalcOpFunc(math::Rational)>(
		// @formatter:on
			1,
			[](Calculator<math::Rational> * calc, CalcASTElem src,
			   bool validate_only = false) -> math::Rational {
				auto * rcalc = reinterpret_cast<RationalCalculator *>(calc);

				if (rcalc->precision < 0) {
					throw CalcASTException(src.token,
					                       L"Can't calculate perfectly"
					                       L" (precision must be set, see :help"
					                       L" :prec)");
				}

				if (!validate_only) {
					try {
						return Irrational::sqrt(
							static_cast<size_t>(rcalc->precision),
							calc->execute_ast(src.children[0]));
					} catch (std::exception & e) {
						throw CalcASTException(calc->get_token(src.children[0]),
						                       LD::s2wstr(
							                       std::string(e.what())));
					}
				} else {
					return rcalc->execute_ast(src.children[0], validate_only);
				}
			}
		));

	help_pages[L"mean()"] =
		L"mean() calculates the mean of a set of elements.\n"
		L"Usage: mean(<element,...>)\n"
		L"Example: mean(10, 20) - calculates the mean of 10 and 20. Returns"
		L" 15.";

	variadic_funcs[L"mean"] =
		[](Calculator<math::Rational> * calc, CalcASTElem src,
		   bool validate_only = false) -> math::Rational {
			std::valarray<math::Rational> elems(math::Rational(0),
			                                    src.children.size());

			for (int i = 0; i < src.children.size(); i++) {
				elems[i] = calc
					->execute_ast(src.children[i], validate_only);
			}

			return elems.sum() / static_cast<int>(elems.size());
		};

	help_pages[L"stdvar()"] =
		L"stdvar() calculates the standard variance of a set of elements.\n"
		L"Usage: stdvar(<sample/population>, <element,...>)\n"
		L"Example: sqrt(stdvar(population, 10, 2, 38, 23, 38, 23, 21)) -"
		L" calculates the standard deviation (sqrt of variance) of the"
		L" population {10, 2, 38, 23, 38, 23, 21}. Returns approximately"
		L" 12.29899614287479072189.";

	variadic_funcs[L"stdvar"] =
		[](Calculator<math::Rational> * calc, CalcASTElem src,
		   bool validate_only = false) -> math::Rational {
			if (src.children.empty() || (
				src.children[0].token.data != L"sample" &&
				src.children[0].token.data != L"population"
			)) {
				throw CalcASTException(src.token,
				                       L"Type must be 'sample' or"
				                       L" 'population'");
			}

			Token type = src.children[0].token;

			if (src.children.size() < 2) {
				throw CalcASTException(src.token,
				                       L"Can't calculate standard variance of"
				                       L" nothing");
			} else if (type.data == L"sample" && src.children.size() < 3) {
				throw CalcASTException(type,
				                       L"Samples must have at least two"
				                       L" elements");
			}

			src.children.erase(src.children.begin());

			std::valarray<math::Rational> elems(math::Rational(0),
			                                    src.children.size());

			for (int i = 0; i < src.children.size(); i++) {
				elems[i] = calc
					->execute_ast(src.children[i], validate_only);
			}

			if (!validate_only) {
				math::Rational mean =
					               elems.sum() /
					               static_cast<int>(elems.size());

				elems -= mean;
				elems *= elems;

				return elems.sum() / static_cast<int>(elems.size() -
				                                      (
					                                      type.data ==
					                                      L"sample"
				                                      ));
			}

			return 0.5;
		};
}

void RationalCalculator::generate_functions_help() {
	std::wstring funcs_help =
		             L"This calculator also has a few functions to help you.\n"
		             L"Each function has its own help page. Functions can be"
		             L" used as a value wherever a value is accepted. You can"
		             L" use them in equations or even in other functions."
		             L" Here's a list of function help pages:\n\n";

	for (auto & page : help_pages) {
		if (page.first.substr(page.first.length() - 2) == L"()") {
			funcs_help.append(page.first + L", ");
		}
	}

	help_pages[L"functions"] = funcs_help
		.substr(0, funcs_help.length() - 2);
	help_pages.at(L"starthere").append(L"\n:help functions");
}

#endif //CALCULATOR_RATIONALCALCULATOR_CPP
