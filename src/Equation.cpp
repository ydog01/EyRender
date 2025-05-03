#include "Equation.hpp"

eval::evaluator<char, double> Equation::evaluator = eval_init::create_real_eval<double>();