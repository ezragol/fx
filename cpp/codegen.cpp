#include "codegen.h"

Value *CodeGen::GenNumberLiteral(NumberLiteral Num)
{
    if (Num.isFloating())
    {
        return ConstantFP::get(*TheContext, APFloat(Num.getFloatVal()));
    }
    else {
        const int & intVal = Num.getIntVal();
        uint8_t bits = floor(log2(intVal)) + 1;
        return ConstantInt::get(*TheContext, APInt(bits, intVal));
    }
}

Value *CodeGen::VariableDefinition(VariableDefinition Var)
{
    vector<AllocaInst *> OldBindings;
    Function *ParentFunction = Builder->GetInsertBlock()->getParent();
    
}