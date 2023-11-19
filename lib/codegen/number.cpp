#include "number.h"

NumberLiteral::NumberLiteral(Location location, NumberType value)
    : Expr(Num, location), value(value) {};

const NumberType &NumberLiteral::getValue()
{
    return value;
}

void NumberLiteral::setValue(NumberType value)
{
    value = value;
}