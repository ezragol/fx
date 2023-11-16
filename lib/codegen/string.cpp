#include "string.h"

const string &ast::StringLiteral::GetStringVal()
{
    return StringVal;
}

void ast::StringLiteral::SetStringVal(string Val)
{
    StringVal = Val;
}

void ast::StringLiteral::Print(string Prefix)
{
    dbgs() << Prefix << "string \"" << StringVal << "\"\n";
}

Value *ast::StringLiteral::Gen()
{
    return GetGenerator()->GenStringLiteral(this);
}

// todo??

Type *ast::StringLiteral::GetType()
{
    return Type::Get
}