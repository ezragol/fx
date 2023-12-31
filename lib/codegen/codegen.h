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

#include "number.h"
#include "string.h"
#include "fndef.h"
#include "chain.h"
#include "binop.h"
#include "when.h"
#include "fncall.h"
#include "ref.h"

using namespace llvm;
using namespace std;
using namespace ast;

#define derived(U) (const unique_ptr<U> &)

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
    
    Function *genFunctionDefinition(const unique_ptr<FunctionDefinition> &func);
    Value *genNumberLiteral(const unique_ptr<NumberLiteral> &num);
    Value *genStringLiteral(const unique_ptr<ast::StringLiteral> &string);
    Value *genChainExpression(const unique_ptr<ChainExpression> &chain);
    Value *genBinaryOperation(const unique_ptr<BinaryOperation> &bin);
    Value *genFunctionCall(const unique_ptr<FunctionCall> &call);
    Value *genVariableRef(const unique_ptr<VariableRef> &ref);

    CodeGenError *error;
    void addToError(string message, Location location);

public:
    CodeGen(string targetTriple, TargetMachine *targetMachine);
    ~CodeGen();

    const unique_ptr<LLVMContext> &getContext();

    int runPass(string outFile);
    Function *loadFunction(string name);
    Value *genericGen(const unique_ptr<Expr> &expr);
    void printError();
};

#endif