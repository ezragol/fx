#include "interface.h"

vector<unique_ptr<Expr>> translateExpressionVec(const FFISafeExpr *start, uint8_t len)
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

unique_ptr<WhenExpression> translateWhen(const FFISafeExpr *ptr)
{
    return make_unique<WhenExpression>(
        translateExpression(ptr->when_expression._0),
        translateExpression(ptr->when_expression._1));
}

unique_ptr<Expr> translateExpression(const FFISafeExpr *ptr)
{
    switch (ptr->tag)
    {
    case FFISafeExpr::Tag::NumberLiteral:
    {
        return make_unique<NumberLiteral>(ptr->number_literal._0, ptr->number_literal._1, ptr->number_literal._2);
    }
    case FFISafeExpr::Tag::StringLiteral:
    {
        return make_unique<ast::StringLiteral>(ptr->string_literal._0);
    }
    case FFISafeExpr::Tag::FunctionDefinition:
    {
        string funcName = ptr->function_definition._0;
        vector<string> args = translateStringVec(ptr->function_definition._1, ptr->function_definition._2);
        return make_unique<FunctionDefinition>(funcName, args, translateExpression(ptr->function_definition._3));
    }
    case FFISafeExpr::Tag::ChainExpression:
    {
        const FFISafeExpr *start = ptr->chain_expression._0;
        uintptr_t size = ptr->chain_expression._1 - 1;
        vector<unique_ptr<WhenExpression>> whenVec;

        for (uint8_t i = 0; i < size; i++)
        {
            whenVec.push_back(translateWhen(start + i));
        }
        return make_unique<ChainExpression>(std::move(whenVec), translateExpression(start + size));
    }
    case FFISafeExpr::Tag::BinaryOperation:
    {
        return make_unique<BinaryOperation>(ptr->binary_operation._0,
                                            translateExpression(ptr->binary_operation._1),
                                            translateExpression(ptr->binary_operation._2));
    }
    case FFISafeExpr::Tag::WhenExpression:
    {
        return translateWhen(ptr);
    }
    case FFISafeExpr::Tag::FunctionCall:
    {
        return make_unique<FunctionCall>(
            ptr->function_call._0,
            translateExpressionVec(ptr->function_call._1, ptr->function_call._2));
    }
    case FFISafeExpr::Tag::VariableRef:
    {
        return make_unique<VariableRef>(ptr->variable_ref._0);
    }
    default:
        return nullptr;
    }
}

vector<unique_ptr<Expr>> reGenerateAST(FFISafeExprVec tokens)
{
    vector<unique_ptr<Expr>> tree;
    const FFISafeExpr *next;
    for (int i = 0; i < tokens.len; i++)
    {
        next = tokens.ptr + i;
        tree.push_back(translateExpression(next));
    }
    return tree;
}