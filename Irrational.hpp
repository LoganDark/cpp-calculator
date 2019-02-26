#ifndef CALCULATOR_IRRATIONAL_HPP
#define CALCULATOR_IRRATIONAL_HPP

#include <iostream>
#include <string>

#include "boilerplate/ld_boilerplate.hpp"

struct Irrational {
	/**
	 * Source: https://rosettacode.org/wiki/Pi#C.23
	 */
	struct PiCalculator {
		math::Integer k  = 1,
		              l  = 3,
		              n  = 0,
		              q  = 10,
		              r  = -30,
		              t  = 1,
		              nr = -30;

		public:

		/**
		 * Gets the next digit of pi.
		 *
		 * I don't understand how this works, I'm just afraid to touch it.
		 *
		 * @return
		 */
		math::Unsigned next_digit() {
			while (true) {
				math::Integer tn = t * n;

				if (4 * q + r - t < tn) {
					math::Integer nn = n;
					nr = (r - tn) * 10;
					n  = (q * 3 + r) * 10 / t - 10 * n;
					q *= 10;
					r  = nr;

					return nn.abs();
				} else {
					t *= l;
					nr = (q * 2 + r) * l;
					n  = (q * (k * 7) + 2 + r * l) / t;
					q *= k;
					l += 2;
					k++;
				}

				r = nr;
			}
		}

		PiCalculator() = default;
	};

	/**
	 * @param digits
	 * @return Pi to `digits` digits of precision
	 */
	static math::Rational pi(math::Unsigned digits) {
		PiCalculator calc;
		std::string  built = "3.";

		for (; digits > 0; digits--) {
			built.append(calc.next_digit().to_string());
		}

		return built;
	}

	/**
	 * https://stackoverflow.com/a/3028326
	 *
	 * @param digits
	 * @return Euler's number to `digits` digits of precision
	 */
	static math::Rational e(size_t digits) {
		math::Rational result = 2;
		math::Unsigned fact   = 1;
		math::Unsigned goal   = LD::ipow<math::Unsigned>(10, digits);

		for (math::Unsigned i = 2; fact <= goal; i++) {
			fact *= i;
			result += math::Rational(1, fact);
		}

		return result.round(digits);
	}

	/**
	 * https://www.mathsisfun.com/numbers/golden-ratio.html
	 * @ "The Most Irrational"
	 *
	 * This algorithm is particularly inefficient, but I'm unable to find a
	 * better one explained in plain English.
	 *
	 * @param digits
	 * @return The golden ratio to `digits` of precision
	 */
	static math::Rational golden_ratio(size_t digits) {
		math::Rational result = 1,
		               last   = 0,
		               error  = math::Rational(1, LD::ipow<math::Unsigned>(
			               10, digits));

		while (LD::abs(result - last) > error) {
			last   = result;
			result = 1 + 1 / result;
		}

		return result.round(digits);
		//return result - last;
	}

	/**
	 * https://github.com/possibly-wrong/precision/issues/4#issuecomment-432800154
	 *
	 * @param digits
	 * @param src
	 * @return sqrt(src) to `digits` of accuracy
	 */
	static math::Rational sqrt(size_t digits, const math::Rational & src) {
		if (src == 0) {
			return 0;
		} else if (src < 0) {
			throw std::underflow_error("Can't find square root of negative"
			                           " number");
		}

		math::Rational result = src,
		               last,
		               error  = math::Rational(1, LD::ipow<math::Unsigned>(
			               10, digits));

		while (LD::abs(result - last) > error) {
			last   = result;
			result = (result + src / result) / 2;
		}

		return result.round(digits);
	}

	/**
	 * Taylor series for sine function
	 *
	 * https://en.wikipedia.org/wiki/Taylor_series#Trigonometric_functions
	 *
	 * @param digits
	 * @param src
	 * @return
	 */
	static math::Rational _sin(size_t digits, const math::Rational & src) {
		math::Rational result = src,
		               error  = math::Rational(1, LD::ipow<math::Unsigned>(
			               10, digits));

		math::Rational delta  = 1,
		               next   = 1,
		               fact   = 1,
		               pow    = src,
		               srcsrc = src * src;

		while (delta > error) {
			fact *= next + 1;
			fact *= next + 2;
			pow *= srcsrc;

			delta = pow / fact;

			if (next.numerator() % 4 == 3) {
				LD::log(L"adding\n");
				result += delta;
			} else {
				LD::log(L"subtracting\n");
				result -= delta;
			}

			next += 2;

			LD::log(L"next: " + LD::wtostring(next) +
			        L"\nfact: " + LD::wtostring(fact) +
			        L"\npow: " + LD::wtostring(pow) + L"\n");
		}

		LD::log(LD::wtostring((next - 1) / 2) + L" iterations\n");

		return result.round(digits);
	}
};

#endif //CALCULATOR_IRRATIONAL_HPP