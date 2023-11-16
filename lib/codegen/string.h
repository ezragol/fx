#ifndef __STRING_LITERAL_H
#define __STRING_LITERAL_H

#include "codegen.h"

namespace ast
{

    class StringLiteral : public Expr
    {
        string StringVal;

    public:
        const string &GetStringVal();

        void SetStringVal(string Val);

        void Print(string Prefix) override;
        Value *Gen() override;
        Type *GetType() override;
    };

}

#endif