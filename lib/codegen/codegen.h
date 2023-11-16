#ifndef __CODEGEN_H
#define __CODEGEN_H

#include <map>
#include <string>
#include <memory>
#include <vector>
#include <iostream>
#include <math.h>

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Bitcode/BitcodeWriter.h"

using namespace llvm;
using namespace std;

class CodeGen;

namespace ast
{
    class Expr
    {
        unique_ptr<CodeGen> &generator;

    public:
        Expr(unique_ptr<CodeGen> &generator);
        const unique_ptr<CodeGen> &getGenerator();

        virtual void print(string prefix);
        virtual Value *gen();
        virtual Type *getType();

        virtual ~Expr() = default;
    };

    class NumberLiteral;
    class StringLiteral;
    class FunctionDefinition;
    class ChainExpression;
    class BinaryOperation;
    class WhenExpression;
    class FunctionCall;
    class VariableRef;
}

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
    unique_ptr<LLVMContext> TheContext;
    unique_ptr<Module> TheModule;
    unique_ptr<IRBuilder<>> Builder;
    map<string, AllocaInst *> NamedValues;
    vector<unique_ptr<FunctionDefinition>> FunctionDefs;
    string TargetTriple;
    TargetMachine *TheTargetMachine;
    AllocaInst *CreateEntryBlockAlloca(Function *TheFunction, StringRef VarName);
    Value *GetPredFCmp(const unique_ptr<WhenExpression> &When);

public:
    CodeGen(string TargetTriple, TargetMachine *TheTargetMachine);
    const unique_ptr<LLVMContext> &GetContext();

    int RunPass(string OutFile);
    Function *LoadFunction(string Name);

    Function *GenFunctionDefinition(FunctionDefinition *Func);
    Value *GenNumberLiteral(NumberLiteral *Num);
    Value *GenStringLiteral(ast::StringLiteral *String);
    Value *GenChainExpression(ChainExpression *Chain);
    Value *GenBinaryOperation(BinaryOperation *Bin);
    Value *GenWhenExpression(WhenExpression *When);
    Value *GenFunctionCall(FunctionCall *Call);
    Value *GenVariableRef(VariableRef *Ref);

    virtual ~CodeGen() = default;
};

#endif