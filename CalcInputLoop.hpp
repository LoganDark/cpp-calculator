#ifndef CALCULATOR_CALCINPUTLOOP_HPP
#define CALCULATOR_CALCINPUTLOOP_HPP

#include <string>

#include "boilerplate/ld_boilerplate.hpp"
#include "PrettifyException.hpp"

#define RETURN_IF(x) if (x) return last_result

template <class Calc>
	std::wstring input_loop(Calc calc) {
		/**
		 * Simply a variable re-used multiple times to store what the user inputs
		 */
		std::wstring input;

		/**
		 * To return when the user requests to quit
		 */
		std::wstring last_result;

		while (true) {
			/**
			 * Quit if the user wants to quit, but else store user input in the
			 * [[input]] variable
			 *
			 * `input == "exit"` is only evaluated after [[LD::get_input]] has
			 * finished and if it didn't already return `true`.
			 */
			RETURN_IF(LD::get_input(L"> ", input, true) || input == L":exit");

#ifdef LD_USE_LINENOISE
			/**
			 * Add this to the history so you can use the up arrow to recall past
			 * commands
			 *
			 * Only supported if LD_USE_LINENOISE is specified (i.e. not on Repl.it)
			 */
			linenoise::AddHistory(LD::w2str(input).c_str());
#endif

			try {
				std::vector<std::wstring> results = calc
					.execute_multiple(input);

				if (!results.empty()) {
					last_result = results.back();

					if (!last_result.empty()) {
						LD::log(last_result + L"\n");
					}
				}
			} catch (CalcASTException & exception) {
				LD::log(prettify_exception(input, exception));
			}
		}
	}

#endif //CALCULATOR_CALCINPUTLOOP_HPP
