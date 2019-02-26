#ifndef CALCULATOR_PRETTIFYEXCEPTION_HPP
#define CALCULATOR_PRETTIFYEXCEPTION_HPP

#include "CalcASTException.hpp"

#include <string>

std::wstring prettify_exception(const std::wstring & input,
                                const CalcASTException & exception) {
	/**
	 * Amount of characters on either side of the error location to show
	 *
	 * This is simply an `unsigned long` because the [[std::wstring]] methods
	 * use it.
	 */
	unsigned long context = 5;

	/**
	 * PBKAC: Problem Between Keyboard And Chair
	 */
	Token         pbkac = exception.get_token();
	unsigned long pos   = pbkac.pos;

	/**
	 * The text snippet to display (this will be clipped accordingly)
	 */
	std::wstring to_display = input;

	/**
	 * Avoid showing too much text
	 */
	if (pos > context) {
		to_display = L"..." + input.substr(pos - context);

		/**
		 * account for width of ellipsis
		 */
		pos = context + 3;
	}

	/**
	 * Avoid showing too much text
	 */
	if (to_display.length() > pbkac.data.length() + pos + context) {
		to_display =
			to_display.substr(0, pbkac.data.length() + pos + context) +
			L"...";
	}

	/**
	 * The error message in full
	 */
	std::wstring built;
	built.append(to_display + L"\n");
	built.append(pos, ' ');
	built.append(pbkac.data.length(), '^');

	/**
	 * Show the position since the snippet of text may not be a full context
	 */

	if (pbkac.data.length() > 1) {
		built
			.append(L" (chars " + LD::wtostring(pbkac.pos + 1) + L"-" +
			        LD::wtostring(pbkac.pos + pbkac.data.length()) +
			        L")");
	} else {
		built.append(L" (char " + LD::wtostring(pbkac.pos + 1) + L")");
	}

	built.append(L"\nError: " + exception.get_msg() + L"\n");

	return built;
}

#endif //CALCULATOR_PRETTIFYEXCEPTION_HPP
