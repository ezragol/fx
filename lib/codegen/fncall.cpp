#include "fncall.h"

const string &ast::FunctionCall::getName()
{
    return name;
}

const vector<unique_ptr<Expr>> &FunctionCall::getArgs()
{
    return args;
}

void FunctionCall::setName(string fnName)
{
    name = fnName;
}

void FunctionCall::setArgs(vector<unique_ptr<Expr>> fnArgs)
{
    args = fnArgs;
}

void FunctionCall::print(string prefix)
{
    dbgs() << prefix << name << " (\n";
    for (auto &arg : args)
    {
        arg->print(prefix + "  ");
    }
    dbgs() << prefix << ")\n";
}

Value *FunctionCall::gen()
{
    return getGenerator()->genFunctionCall(this);
}

// fix
Type *FunctionCall::getType()
{
    return getGenerator();
}