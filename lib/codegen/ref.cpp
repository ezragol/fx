#include "ref.h"

VariableRef::VariableRef(Location location, string name)
    : Expr(Ref, location), name(name){};

const string &VariableRef::getName()
{
    return name;
}

void VariableRef::setName(string varName)
{
    name = varName;
}