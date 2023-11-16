#ifndef __FUNCTION_DEFINITION_H
#define __FUNCTION_DEFINITION_H

#include "codegen.h"

namespace ast
{
    class FunctionDefinition : public Expr
    {
        string Name;
        vector<string> Args;
        unique_ptr<Expr> Body;

    public:
        const string &GetName();
        const vector<string> &GetArgs();
        const unique_ptr<Expr> &GetBody();

        void SetName(string FnName);
        void SetArgs(vector<string> FnArgs);
        void SetBody(unique_ptr<Expr> FnBody);

        void Print(string Prefix) override;
        Value *Gen() override;
        Type *GetType() override;
    };
}

#endif