#ifndef __AST_H
#define __AST_H

#include <memory>
#include <llvm/IR/DerivedTypes.h>

#include "interface.h"

using namespace llvm;
using namespace ast;

namespace ast
{
    class Expr
    {
    public:
        virtual ~Expr() = default;
        virtual Value *codegen() = 0;
    };

    class NumberLiteral : public Expr
    {
        bool Floating;
        int IntVal;
        double FloatVal;

    public:
        NumberLiteral(bool Floating, int IntVal, double FloatVal);
        Value *codegen() override;
    };

    class StringLiteral : public Expr
    {
        string StringVal;

    public:
        StringLiteral(string StringVal);
        Value *codegen() override;
    };

    class VariableDefinition : public Expr {
        string Name;
        unique_ptr<Expr> Definition;
    public:
        VariableDefinition(string Name, unique_ptr<Expr> Definition);
        Value *codegen() override;
    };

    class FunctionDefinition : public Expr {
        string Name;
        vector<string> Args;
        unique_ptr<Expr> Body;
    public:
        FunctionDefinition(string Name, vector<string> Args, unique_ptr<Expr> Body);
        Value *codegen() override;
    };

    class ChainExpression : public Expr {
        vector<unique_ptr<Expr>> Expressions;
    public:
        ChainExpression(vector<unique_ptr<Expr>> Expressions);
        Value *codegen() override;
    };

    class BinaryOperation : public Expr {
        uint8_t Op;
        unique_ptr<Expr> Left;
        unique_ptr<Expr> Right;
    public:
        BinaryOperation(uint8_t Op, unique_ptr<Expr> Left, unique_ptr<Expr> Right);
        Value *codegen() override;
    };

    class WhenExpression : public Expr {
        unique_ptr<Expr> Result;
        unique_ptr<Expr> Predicate;
    public:
        WhenExpression(unique_ptr<Expr> Result, unique_ptr<Expr> Predicate);
        Value *codegen() override;
    };

    class FunctionCall : public Expr {
        string Name;
        vector<unique_ptr<Expr>> Args;
    public:
        FunctionCall(string Name, vector<unique_ptr<Expr>> Args);
        Value *codegen() override;
    };

    class VariableRef : public Expr {
        string Name;
    public:
        VariableRef(string Name);
        Value *codegen() override;
    };
}

#endif