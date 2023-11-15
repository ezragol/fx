#ifndef __CODEGEN_H
#define __CODEGEN_H

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

#include <map>
#include <string>
#include <memory>
#include <vector>
#include <iostream>
#include <math.h>

using namespace llvm;
using namespace std;

class CodeGen;

namespace ast
{
    class Expr
    {
        unique_ptr<CodeGen> &Generator;

    public:
        Expr(unique_ptr<CodeGen> &Generator);
        const unique_ptr<CodeGen> &GetGenerator();

        virtual void Print(string Prefix);
        virtual Value *Gen();
        virtual Type *GetReturnType();

        virtual ~Expr() = default;
    };

    class FunctionDefinition : public Expr
    {
        string Name;
        vector<string> Args;
        unique_ptr<Expr> Body;
        Type *ReturnType;

    public:
        FunctionDefinition(string Name, vector<string> Args, unique_ptr<Expr> Body);
        const string &GetName();
        const vector<string> &GetArgs();
        const unique_ptr<Expr> &GetBody();

        void Print(string Prefix) override;
        Value *Gen() override;
    };

    class BinaryOperation : public Expr
    {
        uint8_t Op;
        unique_ptr<Expr> Left;
        unique_ptr<Expr> Right;

    public:
        BinaryOperation(uint8_t Op, unique_ptr<Expr> Left, unique_ptr<Expr> Right);
        const uint8_t &GetOp();
        const unique_ptr<Expr> &GetLeft();
        const unique_ptr<Expr> &GetRight();

        void Print(string Prefix) override;
        Value *Gen() override;
    };

    class WhenExpression : public Expr
    {
        unique_ptr<Expr> Predicate;
        unique_ptr<Expr> Result;
        Type *ReturnType;

    public:
        WhenExpression(unique_ptr<Expr> Predicate, unique_ptr<Expr> Result);
        const unique_ptr<Expr> &GetPredicate();
        const unique_ptr<Expr> &GetResult();

        void Print(string Prefix) override;
        Value *Gen() override;
    };

    class ChainExpression : public Expr
    {
        vector<unique_ptr<WhenExpression>> Expressions;
        unique_ptr<Expr> Last;
        Type *ReturnType;

    public:
        ChainExpression(vector<unique_ptr<WhenExpression>> Expressions, unique_ptr<Expr> Last, Type *ReturnType);
        const vector<unique_ptr<WhenExpression>> &GetExpressions();
        const unique_ptr<Expr> &GetLast();

        void Print(string Prefix) override;
        Value *Gen() override;
    };

    class FunctionCall : public Expr
    {
        string Name;
        vector<unique_ptr<Expr>> Args;

    public:
        FunctionCall(string Name, vector<unique_ptr<Expr>> Args);
        const string &GetName();
        const vector<unique_ptr<Expr>> &GetArgs();

        void Print(string Prefix) override;
        Value *Gen() override;
    };

    class VariableRef : public Expr
    {
        string Name;

    public:
        VariableRef(string Name);
        const string &GetName();

        void Print(string Prefix) override;
        Value *Gen() override;
    };
}

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