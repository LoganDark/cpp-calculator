#ifndef CALCULATOR_CALCOPERATIONS_HPP
#define CALCULATOR_CALCOPERATIONS_HPP

#include "Calculator.hpp"
#include "CalcASTElem.hpp"

template <class Num>
	struct CalcOperations {
		struct Binary {
			/**
			 * Exponentiation (^) operator.
			 *
			 * @param calc The calculator this is being executed by.
			 * @param src The AST element this is being executed on.
			 * @return
			 */
			static Num exponentiation(Calculator<Num> * calc, CalcASTElem src,
			                          bool validate_only);

			/**
			 * Multiplication (*) operator.
			 *
			 * @param calc The calculator this is being executed by.
			 * @param src The AST element this is being executed on.
			 * @return
			 */
			static Num multiplication(Calculator<Num> * calc, CalcASTElem src,
			                          bool validate_only);

			/**
			 * Division (/) operator.
			 *
			 * @param calc The calculator this is being executed by.
			 * @param src The AST element this is being executed on.
			 * @return
			 */
			static Num division(Calculator<Num> * calc, CalcASTElem src,
			                    bool validate_only);

			/**
			 * Addition (+) operator.
			 *
			 * @param calc The calculator this is being executed by.
			 * @param src The AST element this is being executed on.
			 * @return
			 */
			static Num addition(Calculator<Num> * calc, CalcASTElem src,
			                    bool validate_only);

			/**
			 * Subtraction operator.
			 *
			 * @param calc The calculator this is being executed by.
			 * @param src The AST element this is being executed on.
			 * @return
			 */
			static Num subtraction(Calculator<Num> * calc, CalcASTElem src,
			                       bool validate_only);

			/**
			 * Assigns something to a variable.
			 *
			 * @param calc The calculator this is being executed by.
			 * @param src The AST element this is being executed on.
			 * @return
			 */
			static Num assignment(Calculator<Num> * calc, CalcASTElem src,
			                      bool validate_only);
		};

		struct Unary {
			/**
			 * Calculates the factorial of a number.
			 *
			 * @param calc The calculator this is being executed by.
			 * @param src The AST element this is being executed on.
			 * @return
			 */
			static Num factorial(Calculator<Num> * calc, CalcASTElem src,
			                     bool validate_only);

			/**
			 * Calculates the double factorial of a number.
			 *
			 * @param calc The calculator this is being executed by.
			 * @param src The AST element this is being executed on.
			 * @return
			 */
			static Num dbl_factorial(Calculator<Num> * calc, CalcASTElem src,
			                         bool validate_only);

			/**
			 * Calculates the super factorial of a number.
			 *
			 * https://en.wikipedia.org/wiki/Factorial#Pickover%E2%80%99s_superfactorial
			 *
			 * @param calc The calculator this is being executed by.
			 * @param src The AST element this is being executed on.
			 * @return
			 */
			static Num super_factorial(Calculator<Num> * calc, CalcASTElem src,
			                           bool validate_only);

			/**
			 * Negating a number. Negative numbers are actually numbers plus the
			 * negation operator.
			 *
			 * @param calc The calculator this is being executed by.
			 * @param src The AST element this is being executed on.
			 * @return
			 */
			static Num negation(Calculator<Num> * calc, CalcASTElem src,
			                    bool validate_only);

			/**
			 * I really don't have a better name for this. Besides, +5 is
			 * useless anyway.
			 *
			 * @param calc The calculator this is being executed by.
			 * @param src The AST element this is being executed on.
			 * @return
			 */
			static Num plus(Calculator<Num> * calc, CalcASTElem src,
			                bool validate_only);
		};
	};

#endif //CALCULATOR_CALCOPERATIONS_HPP
