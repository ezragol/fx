#ifndef __AST_H
#define __AST_H

#include <string>
#include <memory>
#include <vector>

using namespace std;

namespace ast
{
    class Expr
    {
    public:
        virtual ~Expr() = default;
    };

    class NumberLiteral : public Expr
    {
        bool Floating;
        int IntVal;
        double FloatVal;

    public:
        NumberLiteral(bool Floating, int IntVal, double FloatVal);
        const bool &isFloating();
        const int &getIntVal();
        const double &getFloatVal();
    };

    // in the future :D
    // class StringLiteral : public Expr
    // {
    //     string StringVal;

    // public:
    //     StringLiteral(string StringVal);
    //     Value *codegen() override;
    // };

    class VariableDefinition : public Expr {
        string Name;
        unique_ptr<Expr> Definition;
    public:
        VariableDefinition(string Name, unique_ptr<Expr> Definition);
        const string &getName();
        const unique_ptr<Expr> &getDefinition();
    };

    class FunctionDefinition : public Expr {
        string Name;
        vector<string> Args;
        unique_ptr<Expr> Body;
    public:
        FunctionDefinition(string Name, vector<string> Args, unique_ptr<Expr> Body);
        const string &getName();
        const vector<string> &getArgs();
        const unique_ptr<Expr> &getBody();
    };

    class ChainExpression : public Expr {
        vector<unique_ptr<Expr>> Expressions;
    public:
        ChainExpression(vector<unique_ptr<Expr>> Expressions);
        const vector<unique_ptr<Expr>> &getExpressions();
    };

    class BinaryOperation : public Expr {
        uint8_t Op;
        unique_ptr<Expr> Left;
        unique_ptr<Expr> Right;
    public:
        BinaryOperation(uint8_t Op, unique_ptr<Expr> Left, unique_ptr<Expr> Right);
        const uint8_t &getOp();
        const unique_ptr<Expr> &getLeft();
        const unique_ptr<Expr> &getRight();
    };

    class WhenExpression : public Expr {
        unique_ptr<Expr> Result;
        unique_ptr<Expr> Predicate;
    public:
        WhenExpression(unique_ptr<Expr> Result, unique_ptr<Expr> Predicate);
        const unique_ptr<Expr> &getResult();
        const unique_ptr<Expr> &getPredicate();
    };

    class FunctionCall : public Expr {
        string Name;
        vector<unique_ptr<Expr>> Args;
    public:
        FunctionCall(string Name, vector<unique_ptr<Expr>> Args);
        const string &getName();
        const vector<unique_ptr<Expr>> &getArgs();
    };

    class VariableRef : public Expr {
        string Name;
    public:
        VariableRef(string Name);
        const string &getName();
    };
}

#endif