#include "interface.h"

vector<FFISafeExpr *> TranslateRustVec(FFISafeExprVec rustVec)
{
    vector<FFISafeExpr *> vec;
    for (int i = 0; i < rustVec.len; i++)
    {
        vec.push_back(rustVec.ptr + i);
    }
    return vec;
}

vector<FFISafeExpr *> TranslateVecPtr(FFISafeExpr *ptr, uint32_t len)
{
    FFISafeExprVec rustVec = {ptr, len};
    return TranslateRustVec(rustVec);
}

vector<string> TranslateStringVec(const char *const *ptr, uint32_t len)
{
    vector<string> vec;
    for (int i = 0; i < len; i++)
    {
        string cpp_str(*(ptr + i));
        vec.push_back(cpp_str);
    }
    return vec;
}

int main()
{
    FFISafeExprVec tokens = recieve_tokens();
    vector<FFISafeExpr *> vec = TranslateRustVec(tokens);
    for (auto item : vec)
    {
        if (item->tag == FFISafeExpr::Tag::FunctionDefinition)
        {
            cout << item->function_definition._0 << "(";
            vector<string> args = TranslateStringVec(item->function_definition._1, item->function_definition._2);
            for (string arg : args)
            {
                cout << arg << ", ";
            }
            cout << ")\n";
        }
    }
}