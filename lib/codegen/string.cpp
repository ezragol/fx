#include "string.h"

ast::StringLiteral::StringLiteral(string stringVal)
    : Expr(Str), stringVal(stringVal){};

const string &ast::StringLiteral::getStringVal()
{
    return stringVal;
}

void ast::StringLiteral::setStringVal(string val)
{
    stringVal = val;
}