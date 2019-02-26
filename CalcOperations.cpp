#ifndef CALCULATOR_CALCOPERATIONS_CPP
#define CALCULATOR_CALCOPERATIONS_CPP

#include "CalcOperations.hpp"
#include "Calculator.hpp"
#include "CalcASTElem.hpp"
#include "CalcASTEnum.hpp"
#include "CalcASTException.hpp"

#include "boilerplate/ld_boilerplate.hpp"

template <class Num>
	Num CalcOperations<Num>::Binary::exponentiation(Calculator<Num> * calc,
	                                                CalcASTElem src,
	                                                bool validate_only) {
		CalcASTElem lhs       = src.children[0];
		CalcASTElem rhs       = src.children[1];
		Num         rhs_final = calc->execute_ast(rhs, validate_only);

		/**
		 * Easy way out, just return 1. Anything to the power of 0 is
		 * always 1.
		 */
		if (rhs_final == 0) {
			if (validate_only) {
				calc->execute_ast(lhs, validate_only);
			}

			return 1;
		}

		/**
		 * Fractional exponents may not be rational, and our
		 * arbitrary-precision types can only represent rational numbers
		 */
		if (!calc->is_int(rhs_final) || rhs_final < 0) {
			throw CalcASTException(calc->get_token(rhs),
			                       L"Powers must be a positive integer");
		}

		if (validate_only) {
			return calc->execute_ast(lhs, validate_only);
		}

		Num lhs_final = calc->execute_ast(lhs);

		return math::Rational(
			LD::ipow(lhs_final.numerator(), rhs_final.numerator()),
			LD::ipow(lhs_final.denominator(), rhs_final.numerator()));
	}

template <class Num>
	Num CalcOperations<Num>::Binary::multiplication(Calculator<Num> * calc,
	                                                CalcASTElem src,
	                                                bool validate_only) {
		CalcASTElem lhs = src.children[0];
		CalcASTElem rhs = src.children[1];

		if (validate_only) {
			calc->execute_ast(lhs, validate_only);

			return calc->execute_ast(rhs, validate_only);
		}

		return calc->execute_ast(lhs) * calc->execute_ast(rhs);
	}

template <class Num>
	Num CalcOperations<Num>::Binary::division(Calculator<Num> * calc,
	                                          CalcASTElem src,
	                                          bool validate_only) {
		CalcASTElem lhs       = src.children[0];
		CalcASTElem rhs       = src.children[1];
		Num         rhs_final = calc->execute_ast(rhs, validate_only);

		if (rhs_final == 0) {
			throw CalcASTException(calc->get_token(src.children[1]),
			                       L"Can't divide by 0");
		}

		if (validate_only) {
			return calc->execute_ast(lhs, validate_only);
		}

		return calc->execute_ast(lhs) / rhs_final;
	}

template <class Num>
	Num CalcOperations<Num>::Binary::addition(Calculator<Num> * calc,
	                                          CalcASTElem src,
	                                          bool validate_only) {
		CalcASTElem lhs = src.children[0];
		CalcASTElem rhs = src.children[1];

		if (validate_only) {
			calc->execute_ast(lhs, validate_only);

			return calc->execute_ast(rhs, validate_only);
		}

		return calc->execute_ast(lhs) + calc->execute_ast(rhs);
	}

template <class Num>
	Num CalcOperations<Num>::Binary::subtraction(Calculator<Num> * calc,
	                                             CalcASTElem src,
	                                             bool validate_only) {
		CalcASTElem lhs = src.children[0];
		CalcASTElem rhs = src.children[1];

		if (validate_only) {
			calc->execute_ast(lhs, validate_only);

			return calc->execute_ast(rhs, validate_only);
		}

		return calc->execute_ast(lhs) - calc->execute_ast(rhs);
	}

template <class Num>
	Num CalcOperations<Num>::Binary::assignment(Calculator<Num> * calc,
	                                            CalcASTElem src,
	                                            bool validate_only) {
		CalcASTElem lhs = src.children[0];
		CalcASTElem rhs = src.children[1];

		/**
		 * 3 = 3.141592653589793
		 *
		 * I don't think so
		 */
		if (lhs.token.type != TK_VARIABLE) {
			throw CalcASTException(calc->get_token(lhs),
			                       L"Can't assign to non-variable");
		} else if (lhs.token.data == L"_") {
			throw CalcASTException(calc->get_token(lhs),
			                       L"Assignment will be overwritten");
		}

		if (validate_only) return calc->execute_ast(rhs, validate_only);

		return calc->variables[lhs.token.data] = calc->execute_ast(rhs);
	}

template <class Num>
	Num CalcOperations<Num>::Unary::factorial(Calculator<Num> * calc,
	                                          CalcASTElem src,
	                                          bool validate_only) {
		CalcASTElem num       = src.children[0];
		Num         num_final = calc->execute_ast(num, validate_only);

		if (!calc->is_int(num_final)) {
			throw CalcASTException(calc->get_token(num),
			                       L"Can't calculate factorial of non-integer");
		} else if (num_final < 0) {
			throw CalcASTException(calc->get_token(num),
			                       L"Can't calculate factorial of negative"
			                       L" integer");
		}

		if (validate_only) return 1;

		Num factored = 1;

		for (Num i(1); i <= num_final; i++) {
			factored *= i;
		}

		return factored;
	}

template <class Num>
	Num CalcOperations<Num>::Unary::dbl_factorial(Calculator<Num> * calc,
	                                              CalcASTElem src,
	                                              bool validate_only) {
		CalcASTElem num       = src.children[0];
		Num         num_final = calc->execute_ast(num, validate_only);

		if (!calc->is_int(num_final)) {
			throw CalcASTException(calc->get_token(num),
			                       L"Can't calculate double factorial of non-"
			                       L"integer");
		} else if (num_final < -1) {
			throw CalcASTException(calc->get_token(num),
			                       L"Can't calculate double factorial of"
			                       L" integer below -1");
		}

		if (validate_only) return 1;

		Num factored = 1;

		for (Num i(1); i <= num_final; i++) {
			if (i.numerator() % 2 == num_final.numerator() % 2) {
				factored *= i;
			}
		}

		return factored;
	}

template <class Num>
	Num CalcOperations<Num>::Unary::super_factorial(Calculator<Num> * calc,
	                                                CalcASTElem src,
	                                                bool validate_only) {
		CalcASTElem num       = src.children[0];
		Num         num_final = calc->execute_ast(num, validate_only);

		if (!calc->is_int(num_final)) {
			throw CalcASTException(calc->get_token(num),
			                       L"Can't calculate super factorial of non-"
			                       L"integer");
		} else if (num_final < 0) {
			throw CalcASTException(calc->get_token(num),
			                       L"Can't calculate super factorial of"
			                       L" negative integer");
		}

		if (validate_only) return 1;

		Num factored  = 1;
		Num factorial = 1;

		for (Num i(1); i <= num_final; i++) {
			factorial *= i;
			factored *= factorial;
		}

		return factored;
	}

template <class Num>
	Num CalcOperations<Num>::Unary::negation(Calculator<Num> * calc,
	                                         CalcASTElem src,
	                                         bool validate_only) {
		return -calc->execute_ast(src.children[0], validate_only);
	}

template <class Num>
	Num CalcOperations<Num>::Unary::plus(Calculator<Num> * calc,
	                                     CalcASTElem src, bool validate_only) {
		return calc->execute_ast(src.children[0], validate_only);
	}

#endif //CALCULATOR_CALCOPERATIONS_CPP
