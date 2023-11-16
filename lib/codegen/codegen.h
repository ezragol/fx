#ifndef __CODEGEN_H
#define __CODEGEN_H

#include "number.h"
#include "string.h"
#include "fndef.h"
#include "chain.h"
#include "binop.h"
#include "when.h"
#include "fncall.h"
#include "ref.h"

using namespace ast;

class CodeGen
{
    unique_ptr<LLVMContext> context;
    unique_ptr<Module> llvmModule;
    unique_ptr<IRBuilder<>> builder;
    map<string, AllocaInst *> namedValues;
    vector<unique_ptr<FunctionDefinition>> functionDefs;
    string targetTriple;
    TargetMachine *targetMachine;
    AllocaInst *createEntryBlockAlloca(Function *function, StringRef varName);
    Value *getPredFCmp(const unique_ptr<WhenExpression> &when);

public:
    CodeGen(string targetTriple, TargetMachine *targetMachine);
    const unique_ptr<LLVMContext> &getContext();

    int runPass(string outFile);
    Function *loadFunction(string name);

    Function *genFunctionDefinition(FunctionDefinition *func);
    Value *genNumberLiteral(NumberLiteral *num);
    Value *genStringLiteral(ast::StringLiteral *string);
    Value *genChainExpression(ChainExpression *chain);
    Value *genBinaryOperation(BinaryOperation *bin);
    Value *genWhenExpression(WhenExpression *when);
    Value *genFunctionCall(FunctionCall *call);
    Value *genVariableRef(VariableRef *ref);

    virtual ~CodeGen() = default;
};

#endif