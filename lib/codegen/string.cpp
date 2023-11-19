#include "string.h"

ast::StringLiteral::StringLiteral(Location location, string stringVal)
    : Expr(Str, location), stringVal(stringVal){};

const string &ast::StringLiteral::getStringVal()
{
    return stringVal;
}

void ast::StringLiteral::setStringVal(string val)
{
    stringVal = val;
}