#ifndef __AST_H
#define __AST_H

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
using namespace ast;

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
    };
}

#endif