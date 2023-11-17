#ifndef __CHAIN_EXPRESSION_H
#define __CHAIN_EXPRESSION_H

#include <vector>

namespace ast
{
    class WhenExpression;

    class ChainExpression : public Expr
    {
        vector<unique_ptr<WhenExpression>> expressions;
        unique_ptr<Expr> last;

    public:
        ChainExpression(vector<unique_ptr<WhenExpression>> expressions, unique_ptr<Expr> last);

        const vector<unique_ptr<WhenExpression>> &getExpressions();
        const unique_ptr<Expr> &getLast();

        void setExpressions(vector<unique_ptr<WhenExpression>> whenExpressions);
        void setLast(unique_ptr<Expr> lastExpr);

        void print(string prefix) override;
    };
}

#endif