#include "ast.h"

using namespace ast;

NumberLiteral::NumberLiteral(bool Floating, int IntVal, double FloatVal)
    : Floating(Floating), IntVal(IntVal), FloatVal(FloatVal) {}

const bool &NumberLiteral::isFloating()
{
    return Floating;
}

const int &NumberLiteral::getIntVal()
{
    return IntVal;
}

const double &NumberLiteral::getFloatVal()
{
    return FloatVal;
}

VariableDefinition::VariableDefinition(string Name, unique_ptr<Expr> Definition)
    : Name(Name), Definition(move(Definition)) {}

const string &VariableDefinition::getName()
{
    return Name;
}

const unique_ptr<Expr> &VariableDefinition::getDefinition()
{
    return Definition;
}

FunctionDefinition::FunctionDefinition(string Name, vector<string> Args, unique_ptr<Expr> Body)
    : Name(Name), Args(Args), Body(move(Body)) {}

const string &FunctionDefinition::getName()
{
    return Name;
}

const vector<string> &FunctionDefinition::getArgs()
{
    return Args;
}

const unique_ptr<Expr> &FunctionDefinition::getBody()
{
    return Body;
}

ChainExpression::ChainExpression(vector<unique_ptr<Expr>> Expressions)
    : Expressions(move(Expressions)) {}

const vector<unique_ptr<Expr>> &ChainExpression::getExpressions()
{
    return Expressions;
}

BinaryOperation::BinaryOperation(uint8_t Op, unique_ptr<Expr> Left, unique_ptr<Expr> Right)
    : Op(Op), Left(move(Left)), Right(move(Right)) {}

const uint8_t &BinaryOperation::getOp()
{
    return Op;
}

const unique_ptr<Expr> &BinaryOperation::getLeft()
{
    return Left;
}

const unique_ptr<Expr> &BinaryOperation::getRight()
{
    return Right;
}

WhenExpression::WhenExpression(unique_ptr<Expr> Result, unique_ptr<Expr> Predicate)
    : Result(move(Result)), Predicate(move(Predicate)) {}

const unique_ptr<Expr> &WhenExpression::getResult()
{
    return Result;
}

const unique_ptr<Expr> &WhenExpression::getPredicate()
{
    return Predicate;
}

FunctionCall::FunctionCall(string Name, vector<unique_ptr<Expr>> Args)
    : Name(Name), Args(move(Args)) {}

const string &FunctionCall::getName()
{
    return Name;
}

const vector<unique_ptr<Expr>> &FunctionCall::getArgs()
{
    return Args;
}

VariableRef::VariableRef(string Name)
    : Name(Name) {}

const string &VariableRef::getName()
{
    return Name;
}