#ifndef __CODEGEN_H
#define __CODEGEN_H

#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <string>
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
        unique_ptr<CodeGen> generator;

    public:
        const unique_ptr<CodeGen> &getGenerator();

        void setGenerator(unique_ptr<CodeGen> codegen);

        virtual void print(string prefix);
        virtual Value *gen();
        virtual Type *getType();

        virtual ~Expr() = default;
        Expr &operator=(const Expr &) = delete;
    };

    class FunctionDefinition;
    class NumberLiteral;
    class StringLiteral;
    class ChainExpression;
    class BinaryOperation;
    class WhenExpression;
    class FunctionCall;
    class VariableRef;
}

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

#include "number.h"
#include "string.h"
#include "fndef.h"
#include "chain.h"
#include "binop.h"
#include "when.h"
#include "fncall.h"
#include "ref.h"

#endif