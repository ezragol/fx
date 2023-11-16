#ifndef __VARIABLE_REF_H
#define __VARIABLE_REF_H

#include "codegen.h"

namespace ast
{
    class VariableRef : public Expr
    {
        string Name;

    public:
        const string &GetName();

        void SetName(string VarName);

        void Print(string Prefix) override;
        Value *Gen() override;
        Type *GetType() override;
    };
}

#endif