#include "ast.h"

NumberLiteral::NumberLiteral(bool Floating, int IntVal, double FloatVal)
    : Floating(Floating), IntVal(IntVal), FloatVal(FloatVal) {}

Value *NumberLiteral::codegen() {
    
}

StringLiteral::StringLiteral(string StringVal)
    : StringVal(StringVal) {}

VariableDefinition::VariableDefinition(string Name, unique_ptr<Expr> Definition)
    : Name(Name), Definition(move(Definition)) {}

FunctionDefinition::FunctionDefinition(string Name, vector<string> Args, unique_ptr<Expr> Body)
    : Name(Name), Args(Args), Body(move(Body)) {}

ChainExpression::ChainExpression(vector<unique_ptr<Expr>> Expressions)
    : Expressions(move(Expressions)) {}

BinaryOperation::BinaryOperation(uint8_t Op, unique_ptr<Expr> Left, unique_ptr<Expr> Right)
    : Op(Op), Left(move(Left)), Right(move(Right)) {}

WhenExpression::WhenExpression(unique_ptr<Expr> Result, unique_ptr<Expr> Predicate)
    : Result(move(Result)), Predicate(move(Predicate)) {}

FunctionCall::FunctionCall(string Name, vector<unique_ptr<Expr>> Args)
    : Name(Name), Args(move(Args)) {}

VariableRef::VariableRef(string Name)
    : Name(Name) {}