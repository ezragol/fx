#include "interface.h"

Location buildFromPtr(const LocatedFFISafeExpr *ptr)
{
    return Location(ptr->line, ptr->column, ptr->filename);
}

vector<unique_ptr<Expr>> translateExpressionVec(const LocatedFFISafeExpr *start, uint8_t len)
{
    vector<unique_ptr<Expr>> vec;
    for (uint8_t i = 0; i < len; i++)
    {
        vec.push_back(translateExpression(start + i));
    }
    return vec;
}

vector<string> translateStringVec(const char *const *start, uint32_t len)
{
    vector<string> vec;
    for (int i = 0; i < len; i++)
    {
        string deref(*(start + i));
        vec.push_back(deref);
    }
    return vec;
}

unique_ptr<WhenExpression> translateWhen(const LocatedFFISafeExpr *ptr)
{
    const FFISafeExpr expr = ptr->expr;
    Location location = buildFromPtr(ptr);
    return make_unique<WhenExpression>(
        location,
        translateExpression(expr.when_expression._0),
        translateExpression(expr.when_expression._1));
}

unique_ptr<Expr> translateExpression(const LocatedFFISafeExpr *ptr)
{
    const FFISafeExpr expr = ptr->expr;
    Location location = buildFromPtr(ptr);
    switch (expr.tag)
    {
    case FFISafeExpr::Tag::NumberLiteral:
    {
        // TODO
        NumberType type;
        type.type = f64;
        type.value.f64 = expr.number_literal._2;
        return make_unique<NumberLiteral>(location, type);
    }
    case FFISafeExpr::Tag::StringLiteral:
    {
        return make_unique<ast::StringLiteral>(location, expr.string_literal._0);
    }
    case FFISafeExpr::Tag::FunctionDefinition:
    {
        string funcName = expr.function_definition._0;
        vector<string> args = translateStringVec(expr.function_definition._1, expr.function_definition._2);
        return make_unique<FunctionDefinition>(location, funcName, args, translateExpression(expr.function_definition._3));
    }
    case FFISafeExpr::Tag::ChainExpression:
    {
        const LocatedFFISafeExpr *start = expr.chain_expression._0;
        uintptr_t size = expr.chain_expression._1 - 1;
        vector<unique_ptr<WhenExpression>> whenVec;

        for (uint8_t i = 0; i < size; i++)
        {
            whenVec.push_back(translateWhen(start + i));
        }
        return make_unique<ChainExpression>(location, std::move(whenVec), translateExpression(start + size));
    }
    case FFISafeExpr::Tag::BinaryOperation:
    {
        return make_unique<BinaryOperation>(location, expr.binary_operation._0,
                                            translateExpression(expr.binary_operation._1),
                                            translateExpression(expr.binary_operation._2));
    }
    case FFISafeExpr::Tag::WhenExpression:
    {
        return translateWhen(ptr);
    }
    case FFISafeExpr::Tag::FunctionCall:
    {
        return make_unique<FunctionCall>(
            location, expr.function_call._0,
            translateExpressionVec(expr.function_call._1, expr.function_call._2));
    }
    case FFISafeExpr::Tag::VariableRef:
    {
        return make_unique<VariableRef>(location, expr.variable_ref._0);
    }
    default:
        return nullptr;
    }
}

vector<unique_ptr<Expr>> reGenerateAST(FFISafeExprVec tokens)
{
    vector<unique_ptr<Expr>> tree;
    const LocatedFFISafeExpr *next;
    for (int i = 0; i < tokens.len; i++)
    {
        next = tokens.ptr + i;
        tree.push_back(translateExpression(next));
    }
    return tree;
}