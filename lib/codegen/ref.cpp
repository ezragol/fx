#include "ref.h"

const string &VariableRef::GetName()
{
    return Name;
}

void VariableRef::SetName(string VarName)
{
    Name = VarName;
}

void VariableRef::Print(string Prefix)
{
    dbgs() << Prefix << "var " << Name << "\n";
}

Value *VariableRef::Gen()
{
    return GetGenerator()->GenVariableRef(this);
}

// TODO
Type *VariableRef::GetType()
{
    return 
}