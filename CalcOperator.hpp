#ifndef CALCULATOR_CALCOPERATOR_HPP
#define CALCULATOR_CALCOPERATOR_HPP

#include <functional>
#include <utility>

#include "Calculator.hpp"
#include "CalcOpFunc.hpp"

enum BinaryAssociativity : bool {
	ASSOCIATE_L = false,
	ASSOCIATE_R = true
};

template <class Num>
	struct CalcOperator {
		std::wstring        op;
		unsigned            order;
		BinaryAssociativity associativity;
		CalcOpFunc(Num)     func = 0;

		CalcOperator(std::wstring op, unsigned order,
		             BinaryAssociativity associativity, CalcOpFunc(Num) func)
			: op(std::move(op)), order(order), associativity(associativity),
			  func(func) {};
	};

enum UnaryAssociativity : bool {
	UASSOCIATE_BEF = false,
	UASSOCIATE_AFT = true
};

template <class Num>
	struct CalcUOperator {
		std::wstring       op;
		UnaryAssociativity associativity;
		CalcOpFunc(Num)    func = 0;

		CalcUOperator(std::wstring op, UnaryAssociativity associativity,
		              CalcOpFunc(Num) func)
			: op(std::move(op)), associativity(associativity), func(func) {};
	};

#endif //CALCULATOR_CALCOPERATOR_HPP
