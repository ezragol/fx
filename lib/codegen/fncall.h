#ifndef __FUNCTION_CALL_H
#define __FUNCTION_CALL_H

#include "ast.h"

namespace ast
{
    class FunctionCall : public Expr
    {
        string name;
        vector<unique_ptr<Expr>> args;

    public:
        FunctionCall(Location location, string name, vector<unique_ptr<Expr>> args);

        const string &getName();
        const vector<unique_ptr<Expr>> &getArgs();

        void setName(string fnName);
        void setArgs(vector<unique_ptr<Expr>> fnArgs);
    };
}

#endif