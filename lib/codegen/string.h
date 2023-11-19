#ifndef __STRING_LITERAL_H
#define __STRING_LITERAL_H

#include "ast.h"

namespace ast
{
    class StringLiteral : public Expr
    {
        string stringVal;

    public:
        StringLiteral(Location location, string stringVal);

        const string &getStringVal();

        void setStringVal(string val);
    };

}

#endif