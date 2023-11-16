#include "when.h"

const unique_ptr<Expr> &WhenExpression::GetPredicate()
{
    return Predicate;
}

const unique_ptr<Expr> &WhenExpression::GetResult()
{
    return Result;
}

void WhenExpression::SetPredicate(unique_ptr<Expr> BoolPredicate)
{
    Predicate = move(BoolPredicate);
}

void WhenExpression::SetResult(unique_ptr<Expr> ResultExpr)
{
    Result = move(ResultExpr);
}

void WhenExpression::Print(string Prefix)
{
    dbgs() << Prefix << "when {\n";
    Predicate->Print(Prefix + "  ");
    Result->Print(Prefix + "  then ");
    dbgs() << Prefix << "}\n";
}

Value *WhenExpression::Gen()
{
    return GetGenerator()->GenWhenExpression(this);
}

Type *WhenExpression::GetType()
{
    return Result->GetType();
}