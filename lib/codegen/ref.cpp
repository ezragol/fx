#include "ref.h"

VariableRef::VariableRef(string name)
    : name(name){};

const string &VariableRef::getName()
{
    return name;
}

void VariableRef::setName(string varName)
{
    name = varName;
}

void VariableRef::print(string prefix)
{
    dbgs() << prefix << "var " << name << "\n";
}

Value *VariableRef::gen()
{
    return getGenerator()->genVariableRef(this);
}

// TODO
Type *VariableRef::getType()
{
    return 
}