#ifndef CALCULATOR_RATIONALCALCULATOR_HPP
#define CALCULATOR_RATIONALCALCULATOR_HPP

#include <string>

#include "Calculator.hpp"
#include "boilerplate/precision/math_Rational.h"

/**
 * A subclass of [[Calculator]] that deals exclusively with [[math::Rational]]s.
 *
 * Includes some extra commands for controlling precision, as well as the
 * required subclass implementations of [[Calculator::is_int]] and
 * [[Calculator::to_string]].
 */
class RationalCalculator : public Calculator<math::Rational> {
	/**
	 * The precision rationals will be displayed as in
	 * [[RationalCalculator::to_string]].
	 */
	long precision = -1;

	/**
	 * Whether or not to commatize the output (1000.123456 -> 1,000.123456)
	 */
	bool commatize = true;

	/**
	 * Check if a rational is an integer. Clearly, rationals are integers if the
	 * denominator's absolute value is one.
	 *
	 * @param num The number to check,
	 * @return Whether it's an integer.
	 */
	bool is_int(const math::Rational & num) override;

	/**
	 * Commatizes a string. For example, turns 100000 into 100,000 and 1000 into
	 * 1,000
	 *
	 * @param str The string to commatize
	 * @return
	 */
	std::wstring commatize_str(const std::wstring & str);

	/**
	 * Registers commands and their corresponding help pages.
	 */
	void register_commands();

	/**
	 * Adds the `commands` help page and adds it to the `starthere` help page.
	 */
	void generate_commands_help();

	/**
	 * Registers all the custom functions
	 */
	void register_functions();

	/**
	 * Generates help for the functions
	 */
	void generate_functions_help();

	public:
	/**
	 * Constructor. Set up some new commands, make some amends to existing help
	 * pages, create some new ones, etc.
	 */
	RationalCalculator();

	/**
	 * Convert a [[math::Rational]] to a string, using the current
	 * [[RationalCalculator::precision]] setting.
	 *
	 * @param num The number to convert.
	 * @return
	 */
	std::wstring to_string(const math::Rational & num) override;
};

#endif //CALCULATOR_RATIONALCALCULATOR_HPP
