#ifndef CALCULATOR_CALCOPFUNC_HPP
#define CALCULATOR_CALCOPFUNC_HPP

#include <functional>

#include "Calculator.hpp"
#include "CalcASTElem.hpp"

#define CalcOpFunc(Num) std::function<Num(Calculator<Num> *, CalcASTElem, bool)>

#endif //CALCULATOR_CALCOPFUNC_HPP
