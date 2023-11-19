#ifndef __NUMBER_LITERAL_H
#define __NUMBER_LITERAL_H

#include "ast.h"

namespace ast
{
    class NumberLiteral : public Expr
    {
        NumberType value;

    public:
        NumberLiteral(Location location, NumberType value);

        const NumberType &getValue();

        void setValue(NumberType value);
    };
}

#endif