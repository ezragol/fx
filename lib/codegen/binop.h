#ifndef __BINARY_OPERATION_H
#define __BINARY_OPERATION_H

#include "codegen.h"

namespace ast
{
    class BinaryOperation : public Expr
    {
        uint8_t op;
        unique_ptr<Expr> left;
        unique_ptr<Expr> right;

    public:
        const uint8_t &getOp();
        const unique_ptr<Expr> &getLeft();
        const unique_ptr<Expr> &getRight();

        void setOp(uint8_t OpInt);
        void setLeft(unique_ptr<Expr> leftExpr);
        void setRight(unique_ptr<Expr> rightExpr);

        void print(string prefix) override;
        Value *gen() override;
        Type *getType() override;
    };
}

#endif