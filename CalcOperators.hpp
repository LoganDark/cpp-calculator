#ifndef CALCULATOR_CALCOPERATORS_HPP
#define CALCULATOR_CALCOPERATORS_HPP

#include <vector>
#include <map>

#include "CalcOperator.hpp"
#include "CalcOperations.cpp"

template <class Num>
	using BinOps = typename CalcOperations<Num>::Binary;

template <class Num>
	using UnOps = typename CalcOperations<Num>::Unary;

template <class Num>
	struct CalcOperators {
		static std::vector<CalcOperator<Num>> binary_ops;
		static std::vector<CalcUOperator<Num>> unary_ops;

		static std::map<std::wstring, CalcOperator<Num>> binary_op_lookup;
		static std::map<std::wstring, CalcUOperator<Num>> unary_op_lookup;

		static std::wstring binary_op_str;
		static std::wstring unary_op_str;
	};

template <class Op>
	std::map<std::wstring, Op> create_lookup(const std::vector<Op> & ops) {
		std::map<std::wstring, Op> lookup;

		for (const Op & op : ops) {
			/**
			 * I would use `lookup[op.op] = op` but that causes templating
			 * errors
			 */
			lookup.insert(std::make_pair(op.op, op));
		}

		return lookup;
	}

template <class Op>
	std::wstring create_ops_str(const std::vector<Op> & ops) {
		std::wstring ops_str;

		for (const Op & op : ops) {
			ops_str.append(op.op);
		}

		return ops_str;
	}

template <class Num>
	std::vector<CalcOperator<Num>> CalcOperators<Num>::binary_ops = {
		CalcOperator<Num>(L"^", 3, ASSOCIATE_R, BinOps<Num>::exponentiation),
		CalcOperator<Num>(L"*", 2, ASSOCIATE_L, BinOps<Num>::multiplication),
		CalcOperator<Num>(L"/", 2, ASSOCIATE_L, BinOps<Num>::division),
		CalcOperator<Num>(L"+", 1, ASSOCIATE_L, BinOps<Num>::addition),
		CalcOperator<Num>(L"-", 1, ASSOCIATE_L, BinOps<Num>::subtraction),
		CalcOperator<Num>(L"=", 0, ASSOCIATE_R, BinOps<Num>::assignment)
	};

template <class Num>
	std::map<std::wstring, CalcOperator<Num>> CalcOperators<Num>::binary_op_lookup
		= create_lookup(CalcOperators<Num>::binary_ops);

template <class Num>
	std::wstring CalcOperators<Num>::binary_op_str
		= create_ops_str(CalcOperators<Num>::binary_ops);

template <class Num>
	std::vector<CalcUOperator<Num>> CalcOperators<Num>::unary_ops = {
		CalcUOperator<Num>(L"!", UASSOCIATE_AFT, UnOps<Num>::factorial),
		CalcUOperator<Num>(L"!!", UASSOCIATE_AFT, UnOps<Num>::dbl_factorial),
		CalcUOperator<Num>(L"$", UASSOCIATE_AFT, UnOps<Num>::super_factorial),
		CalcUOperator<Num>(L"-", UASSOCIATE_BEF, UnOps<Num>::negation),
		CalcUOperator<Num>(L"+", UASSOCIATE_BEF, UnOps<Num>::plus)
	};

template <class Num>
	std::map<std::wstring, CalcUOperator<Num>> CalcOperators<Num>::unary_op_lookup
		= create_lookup(CalcOperators<Num>::unary_ops);

template <class Num>
	std::wstring CalcOperators<Num>::unary_op_str
		= create_ops_str(CalcOperators<Num>::unary_ops);

#endif //CALCULATOR_CALCOPERATORS_HPP
