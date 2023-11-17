#include "number.h"

NumberLiteral::NumberLiteral(bool floating, int intVal, double floatVal)
    : Expr(Num), floating(floating), intVal(intVal), floatVal(floatVal){};

const bool &NumberLiteral::isFloating()
{
    return floating;
}

const int &NumberLiteral::getIntVal()
{
    return intVal;
}

const double &NumberLiteral::getFloatVal()
{
    return floatVal;
}

void NumberLiteral::setFloating(bool state)
{
    floating = state;
}

void NumberLiteral::setIntVal(int val)
{
    intVal = val;
}

void NumberLiteral::setFloatVal(double val)
{
    floatVal = val;
}