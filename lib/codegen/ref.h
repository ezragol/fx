#ifndef __VARIABLE_REF_H
#define __VARIABLE_REF_H

#include "codegen.h"

namespace ast
{
    class VariableRef : public Expr
    {
        string name;

    public:
        VariableRef(string name);

        const string &getName();

        void setName(string varName);

        void print(string prefix) override;
        Value *gen() override;
        Type *getType() override;
    };
}

#endif