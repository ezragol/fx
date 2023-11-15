#include "numberliteral.h"

const bool &NumberLiteral::IsFloating()
{
    return Floating;
}

const int &NumberLiteral::GetIntVal()
{
    return IntVal;
}

const double &NumberLiteral::GetFloatVal()
{
    return FloatVal;
}

void NumberLiteral::SetFloating(bool State)
{
    Floating = State;
}

void NumberLiteral::SetIntVal(int Val)
{
    IntVal = Val;
}

void NumberLiteral::SetFloatVal(double Val)
{
    FloatVal = Val;
}

void NumberLiteral::Print(string Prefix)
{
    dbgs() << Prefix << "number { " << Floating << ", " << IntVal << ", " << FloatVal << " }\n";
}

Value *NumberLiteral::Gen()
{
    return GetGenerator()->GenNumberLiteral(this);
}

Type *NumberLiteral::GetReturnType()
{
    LLVMContext &Context = *(GetGenerator()->GetContext());
    if (IsFloating)
        return Type::getDoubleTy(Context);
    return Type::getInt64Ty(Context);
}