#include "fndef.h"

FunctionDefinition::FunctionDefinition(string name, vector<string> args, unique_ptr<Expr> body)
    : name(name), args(args), body(std::move(body)){};

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

void FunctionDefinition::print(string prefix)
{
    dbgs() << prefix << "fn " << name << " ( ";
    for (string arg : args)
    {
        dbgs() << arg << " ";
    }
    dbgs() << ") {\n"
           << prefix;
    body->print(prefix + "  ");
    dbgs() << prefix << "}\n";
}

Value *FunctionDefinition::gen()
{
    return getGenerator()->genFunctionDefinition(this);
}

Type *FunctionDefinition::getType()
{
    return body->getType();
}