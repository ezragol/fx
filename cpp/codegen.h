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
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

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
    public:
        virtual ~Expr() = default;
        virtual void Print(string Prefix) = 0;
        virtual Value *Gen(CodeGen *Generator) = 0;
    };

    class NumberLiteral : public Expr
    {
        bool Floating;
        int IntVal;
        double FloatVal;

    public:
        NumberLiteral(bool Floating, int IntVal, double FloatVal);
        const bool &IsFloating();
        const int &GetIntVal();
        const double &GetFloatVal();
        void Print(string Prefix) override;
        Value *Gen(CodeGen *Generator) override;
    };

    class StringLiteral : public Expr
    {
        string StringVal;

    public:
        StringLiteral(string StringVal);
        const string &GetStringVal();
        void Print(string Prefix) override;
        Value *Gen(CodeGen *Generator) override;
    };

    class VariableDefinition : public Expr
    {
        string Name;
        unique_ptr<Expr> Definition;

    public:
        VariableDefinition(string Name, unique_ptr<Expr> Definition);
        const string &GetName();
        const unique_ptr<Expr> &GetDefinition();
        void Print(string Prefix) override;
        Value *Gen(CodeGen *Generator) override;
    };

    class FunctionDefinition : public Expr
    {
        string Name;
        vector<string> Args;
        unique_ptr<Expr> Body;

    public:
        FunctionDefinition(string Name, vector<string> Args, unique_ptr<Expr> Body);
        const string &GetName();
        const vector<string> &GetArgs();
        const unique_ptr<Expr> &GetBody();
        void Print(string Prefix) override;
        Value *Gen(CodeGen *Generator) override;
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
        Value *Gen(CodeGen *Generator) override;
    };

    class WhenExpression : public Expr
    {
        unique_ptr<Expr> Result;
        unique_ptr<Expr> Predicate;

    public:
        WhenExpression(unique_ptr<Expr> Result, unique_ptr<Expr> Predicate);
        const unique_ptr<Expr> &GetResult();
        const unique_ptr<Expr> &GetPredicate();
        void Print(string Prefix) override;
        Value *Gen(CodeGen *Generator) override;
    };

    class ChainExpression : public Expr
    {
        vector<unique_ptr<WhenExpression>> Expressions;
        unique_ptr<Expr> Last;

    public:
        ChainExpression(vector<unique_ptr<WhenExpression>> Expressions, unique_ptr<Expr> Last);
        const vector<unique_ptr<WhenExpression>> &GetExpressions();
        const unique_ptr<Expr> &GetLast();
        void Print(string Prefix) override;
        Value *Gen(CodeGen *Generator) override;
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
        Value *Gen(CodeGen *Generator) override;
    };

    class VariableRef : public Expr
    {
        string Name;

    public:
        VariableRef(string Name);
        const string &GetName();
        void Print(string Prefix) override;
        Value *Gen(CodeGen *Generator) override;
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
    TargetMachine *TargetMachine;
    AllocaInst *CreateEntryBlockAlloca(Function *TheFunction, StringRef VarName);
    Function *LoadFunction(string Name);
    Value *NestChainExpression(ChainExpression *When, int Index);

public:
    CodeGen(string TargetTriple, class TargetMachine *TargetMachine);
    int RunPass(string OutFile);

    Value *GenNumberLiteral(NumberLiteral *Num);
    Value *GenStringLiteral(ast::StringLiteral *String);
    Value *GenVariableDefinition(VariableDefinition *Var);
    Function *GenFunctionDefinition(FunctionDefinition *Func);
    Value *GenChainExpression(ChainExpression *Chain);
    Value *GenBinaryOperation(BinaryOperation *Bin);
    Value *GenWhenExpression(WhenExpression *When);
    Value *GenFunctionCall(FunctionCall *Call);
    Value *GenVariableRef(VariableRef *Ref);
};

#endif