#include "chain.h"

ChainExpression::ChainExpression(vector<unique_ptr<WhenExpression>> expressions, unique_ptr<Expr> last)
    : expressions(expressions), last(move(last)){};

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
    expressions = whenExpressions;
}

void ChainExpression::setLast(unique_ptr<Expr> lastExpr)
{
    last = move(lastExpr);
}

void ChainExpression::print(string prefix)
{
    dbgs() << prefix << "chain { \n";
    for (auto &expr : expressions)
    {
        expr->print(prefix + "  ");
    }
    last->print(prefix + "  ");
    dbgs() << prefix << "}\n";
}

Value *ChainExpression::gen()
{
    return getGenerator()->genChainExpression(this);
}

Type *ChainExpression::getType()
{
    return last->getType();
}