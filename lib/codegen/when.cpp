#include "when.h"

WhenExpression::WhenExpression(Location location, unique_ptr<Expr> predicate, unique_ptr<Expr> result)
    : Expr(WhenExpr, location), predicate(std::move(predicate)), result(std::move(result)){};

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
    predicate = std::move(boolPredicate);
}

void WhenExpression::setResult(unique_ptr<Expr> resultExpr)
{
    result = std::move(resultExpr);
}