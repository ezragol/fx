#ifndef __FUNCTION_CALL_H
#define __FUNCTION_CALL_H

#include "codegen.h"

namespace ast
{
    class FunctionCall : public Expr
    {
        string name;
        vector<unique_ptr<Expr>> args;

    public:
        const string &getName();
        const vector<unique_ptr<Expr>> &getArgs();

        void setName(string fnName);
        void setArgs(vector<unique_ptr<Expr>> fnArgs);

        void print(string prefix) override;
        Value *gen() override;
        Type *getType() override;
    };
}

#endif