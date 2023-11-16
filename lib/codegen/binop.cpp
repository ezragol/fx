#include "binop.h"

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
    left = move(leftExpr);
}

void BinaryOperation::setRight(unique_ptr<Expr> rightExpr)
{
    right = move(rightExpr);
}

void BinaryOperation::print(string prefix)
{
    dbgs() << prefix << "op " << unsigned(op) << " { \n";
    left->print(prefix + "  ");
    light->print(prefix + "  ");
    dbgs() << prefix << "}\n";
}

Value *BinaryOperation::gen()
{
    return getGenerator()->genBinaryOperation(this);
}

// TODO
Type *BinaryOperation::getType()
{
    
}