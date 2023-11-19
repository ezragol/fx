#include "fncall.h"

FunctionCall::FunctionCall(Location location, string name, vector<unique_ptr<Expr>> args)
    : Expr(FnCall, location), name(name), args(std::move(args)){};

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
    args = std::move(fnArgs);
}