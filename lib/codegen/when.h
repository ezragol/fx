#ifndef __WHEN_EXPRESSION_H
#define __WHEN_EXPRESSION_H

#include "ast.h"

namespace ast
{
    class WhenExpression : public Expr
    {
        unique_ptr<Expr> predicate;
        unique_ptr<Expr> result;

    public:
        WhenExpression(Location location, unique_ptr<Expr> predicate, unique_ptr<Expr> result);

        const unique_ptr<Expr> &getPredicate();
        const unique_ptr<Expr> &getResult();

        void setPredicate(unique_ptr<Expr> boolPredicate);
        void setResult(unique_ptr<Expr> resultExpr);
    };
}

using namespace ast;

#endif