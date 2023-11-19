#include "chain.h"

ChainExpression::ChainExpression(Location location, vector<unique_ptr<WhenExpression>> expressions, unique_ptr<Expr> last)
    : Expr(ChainExpr, location), expressions(std::move(expressions)), last(std::move(last)){};

const vector<unique_ptr<WhenExpression>> &ChainExpression::getExpressions()
{
    return expressions;
}

const unique_ptr<Expr> &ChainExpression::getLast()
{
    return last;
}

void ChainExpression::setExpressions(vector<unique_ptr<WhenExpression>> whenExpressions)
{
    expressions = std::move(whenExpressions);
}

void ChainExpression::setLast(unique_ptr<Expr> lastExpr)
{
    last = std::move(lastExpr);
}