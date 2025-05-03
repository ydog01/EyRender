#pragma once
#include <string>
#include "eval_init.hpp"

enum class RelationalOperator : int
{
    EQUAL,
    NOT_EQUAL,
    LESS_THAN,
    LESS_THAN_OR_EQUAL,
    GREATER_THAN,
    GREATER_THAN_OR_EQUAL,
    INVALID
};

struct Equation
{
    static eval::evaluator<char,double> evaluator;
    std::string expression;
    RelationalOperator type;
    eval::epre<double> value;//left - right
};