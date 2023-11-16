#include "number.h"

NumberLiteral::NumberLiteral(bool floating, int intVal, double floatVal)
    : floating(floating), intVal(intVal), floatVal(floatVal){};

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

void NumberLiteral::print(string prefix)
{
    dbgs() << prefix << "number { " << floating << ", " << intVal << ", " << floatVal << " }\n";
}

Value *NumberLiteral::gen()
{
    return getGenerator()->genNumberLiteral(this);
}

Type *NumberLiteral::getType()
{
    LLVMContext &context = *(getGenerator()->getContext());
    if (floating)
        return Type::getDoubleTy(context);
    return Type::getInt64Ty(context);
}