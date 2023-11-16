#ifndef __WHEN_EXPRESSION_H
#define __WHEN_EXPRESSION_H

#include "codegen.h"

namespace ast
{
    class WhenExpression : public Expr
    {
        unique_ptr<Expr> Predicate;
        unique_ptr<Expr> Result;

    public:
        const unique_ptr<Expr> &GetPredicate();
        const unique_ptr<Expr> &GetResult();

        void SetPredicate(unique_ptr<Expr> BoolPredicate);
        void SetResult(unique_ptr<Expr> ResultExpr);

        void Print(string Prefix) override;
        Value *Gen() override;
        Type *GetType() override;
    };
}

#endif