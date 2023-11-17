#include "fndef.h"

FunctionDefinition::FunctionDefinition(string name, vector<string> args, unique_ptr<Expr> body)
    : Expr(FnDef), name(name), args(args), body(std::move(body)){};

const string &FunctionDefinition::getName()
{
    return name;
}

const vector<string> &FunctionDefinition::getArgs()
{
    return args;
}

const unique_ptr<Expr> &FunctionDefinition::getBody()
{
    return body;
}

void FunctionDefinition::setName(string fnName)
{
    name = fnName;
}

void FunctionDefinition::setArgs(vector<string> fnArgs)
{
    args = fnArgs;
}

void FunctionDefinition::setBody(unique_ptr<Expr> fnBody)
{
    body = std::move(fnBody);
}