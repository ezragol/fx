#include "string.h"

ast::StringLiteral::StringLiteral(string stringVal)
    : stringVal(stringVal){};

const string &ast::StringLiteral::getStringVal()
{
    return stringVal;
}

void ast::StringLiteral::setStringVal(string val)
{
    stringVal = val;
}

void ast::StringLiteral::print(string prefix)
{
    dbgs() << prefix << "string \"" << stringVal << "\"\n";
}

Value *ast::StringLiteral::gen()
{
    return getGenerator()->genStringLiteral(this);
}

// TOOD
Type *ast::StringLiteral::getType()
{
    // return Type::get
}