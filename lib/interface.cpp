#include "interface.h"

vector<unique_ptr<Expr>> TranslateExpressionVec(const FFISafeExpr *Start, uint8_t Len)
{
    vector<unique_ptr<Expr>> Vec;
    for (uint8_t i = 0; i < Len; i++)
    {
        Vec.push_back(TranslateExpression(Start + i));
    }
    return Vec;
}

vector<string> TranslateStringVec(const char *const *Start, uint32_t Len)
{
    vector<string> Vec;
    for (int i = 0; i < Len; i++)
    {
        string Deref(*(Start + i));
        Vec.push_back(Deref);
    }
    return Vec;
}

unique_ptr<WhenExpression> TranslateWhen(const FFISafeExpr *Ptr)
{
    return make_unique<WhenExpression>(
        TranslateExpression(Ptr->when_expression._0),
        TranslateExpression(Ptr->when_expression._1));
}

unique_ptr<Expr> TranslateExpression(const FFISafeExpr *Ptr)
{
    switch (Ptr->tag)
    {
    case FFISafeExpr::Tag::NumberLiteral:
    {
        return make_unique<NumberLiteral>(Ptr->number_literal._0, Ptr->number_literal._1, Ptr->number_literal._2);
    }
    case FFISafeExpr::Tag::StringLiteral:
    {
        return make_unique<ast::StringLiteral>(Ptr->string_literal._0);
    }
    case FFISafeExpr::Tag::FunctionDefinition:
    {
        string FuncName = Ptr->function_definition._0;
        vector<string> Args = TranslateStringVec(Ptr->function_definition._1, Ptr->function_definition._2);
        return make_unique<FunctionDefinition>(FuncName, Args, TranslateExpression(Ptr->function_definition._3));
    }
    case FFISafeExpr::Tag::ChainExpression:
    {
        const FFISafeExpr *Start = Ptr->chain_expression._0;
        uintptr_t Size = Ptr->chain_expression._1 - 1;
        vector<unique_ptr<WhenExpression>> WhenVec;

        for (uint8_t i = 0; i < Size; i++)
        {
            WhenVec.push_back(TranslateWhen(Start + i));
        }
        return make_unique<ChainExpression>(std::move(WhenVec), TranslateExpression(Start + Size));
    }
    case FFISafeExpr::Tag::BinaryOperation:
    {
        return make_unique<BinaryOperation>(Ptr->binary_operation._0,
                                            TranslateExpression(Ptr->binary_operation._1),
                                            TranslateExpression(Ptr->binary_operation._2));
    }
    case FFISafeExpr::Tag::WhenExpression:
    {
        return TranslateWhen(Ptr);
    }
    case FFISafeExpr::Tag::FunctionCall:
    {
        return make_unique<FunctionCall>(
            Ptr->function_call._0,
            TranslateExpressionVec(Ptr->function_call._1, Ptr->function_call._2));
    }
    case FFISafeExpr::Tag::VariableRef:
    {
        return make_unique<VariableRef>(Ptr->variable_ref._0);
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