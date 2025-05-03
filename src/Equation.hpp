#pragma once
#include <string>

enum class RelationalOperator : int
{
    EQUAL,
    LESS_THAN,
    LESS_THAN_OR_EQUAL,
    GREATER_THAN,
    GREATER_THAN_OR_EQUAL
};

struct Equation
{
    std::string expression;
    RelationalOperator type;
    bool valid = false;
};