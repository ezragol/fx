#include "fndef.h"

const string &FunctionDefinition::GetName()
{
    return Name;
}

const vector<string> &FunctionDefinition::GetArgs()
{
    return Args;
}

const unique_ptr<Expr> &FunctionDefinition::GetBody()
{
    return Body;
}

void FunctionDefinition::SetName(string FnName)
{
    Name = FnName;
}

void FunctionDefinition::SetArgs(vector<string> FnArgs)
{
    Args = FnArgs;
}

void FunctionDefinition::SetBody(unique_ptr<Expr> FnBody)
{
    Body = move(FnBody);
}

void FunctionDefinition::Print(string Prefix)
{
    dbgs() << Prefix << "fn " << Name << " ( ";
    for (string Arg : Args)
    {
        dbgs() << Arg << " ";
    }
    dbgs() << ") {\n"
           << Prefix;
    Body->Print(Prefix + "  ");
    dbgs() << Prefix << "}\n";
}

Value *FunctionDefinition::Gen()
{
    return GetGenerator()->GenFunctionDefinition(this);
}

Type *FunctionDefinition::GetType()
{
    return Body->GetType();
}