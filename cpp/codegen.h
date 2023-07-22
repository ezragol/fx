#ifndef __CODEGEN_H
#define __CODEGEN_H

#include "ast.h"

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
#include <math.h>

using namespace llvm;
using namespace ast;

class CodeGen {
    std::unique_ptr<LLVMContext> TheContext;
    std::unique_ptr<Module> TheModule;
    std::unique_ptr<IRBuilder<>> Builder;
    std::map<std::string, AllocaInst *> NamedValues;
    ExitOnError ExitOnErr;
public:
    Value *GenNumberLiteral(NumberLiteral Num);
    Value *GenVariableDefinition(VariableDefinition Var);
    Value *GenFunctionDefinition(FunctionDefinition Func);
    Value *GenChainExpression(ChainExpression Chain);
    Value *GenBinaryOperation(BinaryOperation Bin);
    Value *GenWhenExpression(WhenExpression When);
    Value *GenFunctionCall(FunctionCall Call);
    Value *GenVariableRef(VariableRef Ref);
};

#endif