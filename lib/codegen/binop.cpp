#include "binop.h"

BinaryOperation::BinaryOperation(uint8_t op, unique_ptr<Expr> left, unique_ptr<Expr> right)
    : Expr(BinaryOp), op(op), left(std::move(left)), right(std::move(right)){};

const uint8_t &BinaryOperation::getOp()
{
    return op;
}

const unique_ptr<Expr> &BinaryOperation::getLeft()
{
    return left;
}

const unique_ptr<Expr> &BinaryOperation::getRight()
{
    return right;
}

void BinaryOperation::setOp(uint8_t opInt)
{
    op = opInt;
}

void BinaryOperation::setLeft(unique_ptr<Expr> leftExpr)
{
    left = std::move(leftExpr);
}

void BinaryOperation::setRight(unique_ptr<Expr> rightExpr)
{
    right = std::move(rightExpr);
}