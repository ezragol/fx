#ifndef __FUNCTION_DEFINITION_H
#define __FUNCTION_DEFINITION_H

#include "codegen.h"

namespace ast
{
    class FunctionDefinition : public Expr
    {
        string name;
        vector<string> args;
        unique_ptr<Expr> body;

    public:
        FunctionDefinition(string name, vector<string> args, unique_ptr<Expr> fnBody);

        const string &getName();
        const vector<string> &getArgs();
        const unique_ptr<Expr> &getBody();

        void setName(string fnName);
        void setArgs(vector<string> fnArgs);
        void setBody(unique_ptr<Expr> fnBody);

        void print(string prefix) override;
        Value *gen() override;
        Type *getType() override;
    };
}

#endif