#include "ref.h"

VariableRef::VariableRef(string name)
    : Expr(Ref), name(name){};

const string &VariableRef::getName()
{
    return name;
}

void VariableRef::setName(string varName)
{
    name = varName;
}