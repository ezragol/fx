#ifndef __NUMBER_LITERAL_H
#define __NUMBER_LITERAL_H

#include "ast.h"

namespace ast
{
    class NumberLiteral : public Expr
    {
        bool floating;
        int intVal;
        double floatVal;

    public:
        NumberLiteral(bool floating, int intVal, double floatVal);

        const bool &isFloating();
        const int &getIntVal();
        const double &getFloatVal();

        void setFloating(bool state);
        void setIntVal(int val);
        void setFloatVal(double val);

        void print(string prefix) override;
        Value *gen() override;
        Type *getType() override;
    };
}

#endif