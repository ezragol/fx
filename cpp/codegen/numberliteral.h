#include "codegen.h"

namespace ast
{
    class NumberLiteral : public Expr
    {
        bool Floating;
        int IntVal;
        double FloatVal;

    public:
        const bool &IsFloating();
        const int &GetIntVal();
        const double &GetFloatVal();

        void SetFloating(bool State);
        void SetIntVal(int Val);
        void SetFloatVal(double Val);

        void Print(string Prefix) override;
        Value *Gen() override;
        Type *GetReturnType() override;
    };
}