#include "interface.h"

vector<unique_ptr<Expr>> TranslateExpressionVec(const FFISafeExpr *Ptr, uint8_t Len)
{
    vector<unique_ptr<Expr>> Vec;
    for (uint8_t i = 0; i < Len; i++)
    {
        Vec.push_back(TranslateExpression(Ptr + i));
    }
    return Vec;
}

vector<string> TranslateStringVec(const char *const *Ptr, uint32_t Len)
{
    vector<string> Vec;
    for (int i = 0; i < Len; i++)
    {
        string Deref(*(Ptr + i));
        Vec.push_back(Deref);
    }
    return Vec;
}

unique_ptr<Expr> TranslateExpression(const FFISafeExpr *Raw)
{
    switch (Raw->tag)
    {
    case FFISafeExpr::Tag::NumberLiteral:
    {
        return make_unique<NumberLiteral>(Raw->number_literal._0, Raw->number_literal._1, Raw->number_literal._2);
    }
    case FFISafeExpr::Tag::StringLiteral:
    {
        return make_unique<ast::StringLiteral>(Raw->string_literal._0);
    }
    case FFISafeExpr::Tag::VariableDefinition:
    {
        return make_unique<VariableDefinition>(
            Raw->variable_definition._0,
            TranslateExpression(Raw->variable_definition._1));
    }
    case FFISafeExpr::Tag::FunctionDefinition:
    {
        string FuncName = Raw->function_definition._0;
        vector<string> Args = TranslateStringVec(Raw->function_definition._1, Raw->function_definition._2);
        return make_unique<FunctionDefinition>(FuncName, Args, TranslateExpression(Raw->function_definition._3));
    }
    case FFISafeExpr::Tag::ChainExpression:
    {
        return make_unique<ChainExpression>(
            TranslateExpressionVec(Raw->chain_expression._0, Raw->chain_expression._1));
    }
    case FFISafeExpr::Tag::BinaryOperation:
    {
        return make_unique<BinaryOperation>(Raw->binary_operation._0,
                                            TranslateExpression(Raw->binary_operation._1),
                                            TranslateExpression(Raw->binary_operation._2));
    }
    case FFISafeExpr::Tag::WhenExpression:
    {
        return make_unique<WhenExpression>(
            TranslateExpression(Raw->when_expression._0),
            TranslateExpression(Raw->when_expression._1));
    }
    case FFISafeExpr::Tag::FunctionCall:
    {
        return make_unique<FunctionCall>(
            Raw->function_call._0,
            TranslateExpressionVec(Raw->function_call._1, Raw->function_call._2));
    }
    case FFISafeExpr::Tag::VariableRef:
    {
        return make_unique<VariableRef>(Raw->variable_ref._0);
    }
    }
}

vector<unique_ptr<Expr>> ReGenerateAST(FFISafeExprVec Tokens)
{
    vector<unique_ptr<Expr>> Tree;
    const FFISafeExpr *Next;
    for (int i = 0; i < Tokens.len; i++)
    {
        Next = Tokens.ptr + i;
        Tree.push_back(TranslateExpression(Next));
    }
    return Tree;
}

void PrintAST(vector<unique_ptr<Expr>> &Tree)
{
    for (auto &branch : Tree)
    {
        branch->Print("");
    }
}