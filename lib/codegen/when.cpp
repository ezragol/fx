#include "when.h"

WhenExpression::WhenExpression(unique_ptr<Expr> predicate, unique_ptr<Expr> result)
    : predicate(move(predicate)), result(move(result)){};

const unique_ptr<Expr> &WhenExpression::getPredicate()
{
    return predicate;
}

const unique_ptr<Expr> &WhenExpression::getResult()
{
    return result;
}

void WhenExpression::setPredicate(unique_ptr<Expr> boolPredicate)
{
    predicate = move(boolPredicate);
}

void WhenExpression::setResult(unique_ptr<Expr> resultExpr)
{
    result = move(resultExpr);
}

void WhenExpression::print(string prefix)
{
    dbgs() << prefix << "when {\n";
    predicate->print(prefix + "  ");
    result->print(prefix + "  then ");
    dbgs() << prefix << "}\n";
}

Value *WhenExpression::gen()
{
    return getGenerator()->genWhenExpression(this);
}

Type *WhenExpression::getType()
{
    return result->getType();
}