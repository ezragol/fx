#include "codegen.h"

const unique_ptr<CodeGen> &Expr::getGenerator()
{
    return generator;
}

void Expr::setGenerator(unique_ptr<CodeGen> codegen)
{
    generator = move(codegen);
}

/////////

CodeGen::CodeGen(string targetTriple, TargetMachine *targetMachine)
    : targetTriple(targetTriple), targetMachine(targetMachine)
{
    context = make_unique<LLVMContext>();
    llvmModule = make_unique<Module>("fx", *context);
    builder = make_unique<IRBuilder<>>(*context);

    llvmModule->setTargetTriple(targetTriple);
    llvmModule->setDataLayout(targetMachine->createDataLayout());
}

const unique_ptr<LLVMContext> &CodeGen::getContext()
{
    return context;
}

int CodeGen::runPass(string outFile)
{
    auto fileType = CGFT_ObjectFile;
    std::error_code ec;
    raw_fd_ostream dest(outFile, ec, sys::fs::OF_None);

    if (ec)
    {
        errs() << "Could not open file: " << ec.message();
        return 1;
    }

    LoopAnalysisManager LAM;
    FunctionAnalysisManager FAM;
    CGSCCAnalysisManager CGAM;
    ModuleAnalysisManager MAM;
    PassBuilder PB(targetMachine);

    PB.registerModuleAnalyses(MAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

    ModulePassManager MPM = PB.buildPerModuleDefaultPipeline(OptimizationLevel::O2);
    MPM.run(*llvmModule, MAM);

    WriteBitcodeToFile(*llvmModule, dest);
    dest.flush();

    // TheModule->print(dbgs(), nullptr);

    return 0;
}

// taken from llvm examples (like most things)
AllocaInst *CodeGen::createEntryBlockAlloca(Function *function, StringRef varName)
{
    IRBuilder<> TmpB(&function->getEntryBlock(),
                     function->getEntryBlock().begin());
    return TmpB.CreateAlloca(Type::getDoubleTy(*context), nullptr, varName);
}

Function *CodeGen::loadFunction(string name)
{
    if (auto *fn = llvmModule->getFunction(name))
    {
        return fn;
    }
    errs() << "can't find function!\n";
    return nullptr;
}

Value *CodeGen::genNumberLiteral(NumberLiteral *num)
{
    if (num->isFloating())
    {
        return ConstantFP::get(*context, APFloat(num->getFloatVal()));
    }
    else
    {
        const int &intVal = num->getIntVal();
        uint8_t bits = floor(log2(intVal)) + 1;
        return ConstantInt::get(*context, APInt(bits, intVal));
    }
}

Function *CodeGen::genFunctionDefinition(FunctionDefinition *def)
{
    // define argument types and return type
    vector<Type *> argT(def->getArgs().size(), Type::getDoubleTy(*context));
    FunctionType *fnType = FunctionType::get(Type::getDoubleTy(*context), argT, false);
    Function *fn = Function::Create(fnType, Function::ExternalLinkage, def->getName(), llvmModule.get());
    BasicBlock *block = BasicBlock::Create(*context, "entry", fn);
    builder->SetInsertPoint(block);
    namedValues.clear();

    // create a "named value" (aka variable) for each argument of the function
    unsigned int i = 0;
    for (auto &arg : fn->args())
    {
        string name = def->getArgs()[i++];
        arg.setName(name);
        AllocaInst *alloca = createEntryBlockAlloca(fn, name);
        builder->CreateStore(&arg, alloca);
        namedValues[name] = alloca;
    }

    if (Value *returned = def->getBody()->gen())
    {
        builder->CreateRet(returned);
        verifyFunction(*fn);
        return fn;
    }

    fn->eraseFromParent();
    errs() << "missing return val!\n";
    return nullptr;
}

// todo
Value *CodeGen::genStringLiteral(ast::StringLiteral *string)
{
    return ConstantFP::get(*context, APFloat(0.0));
}

Value *CodeGen::getPredFCmp(const unique_ptr<WhenExpression> &when)
{
    Value *predicate = when->getPredicate()->gen();
    if (!predicate)
    {
        errs() << "missing predicate!\n";
        return nullptr;
    }
    return builder->CreateFCmpONE(predicate, ConstantFP::get(*context, APFloat(0.0)), "ifcond");
}

// todo
Value *CodeGen::genChainExpression(ChainExpression *chain)
{
    vector<BasicBlock *> blocks;
    vector<Value *> results;

    Value *predicate = getPredFCmp(chain->getExpressions()[0]);
    if (!predicate)
    {
        errs() << "missing predicate!\n";
        return nullptr;
    }

    Function *parent = builder->GetInsertBlock()->getParent();
    BasicBlock *current = BasicBlock::Create(*context, "then", parent);
    BasicBlock *split = BasicBlock::Create(*context, "else", parent);
    BasicBlock *merge = BasicBlock::Create(*context, "ifcont");
    Type *returnType = nullptr;

    builder->CreateCondBr(predicate, current, split);

    const vector<unique_ptr<WhenExpression>> &exprs = chain->getExpressions();
    for (int j = 1; j <= exprs.size(); j++)
    {
        auto &when = exprs[j];
        auto &last = exprs[j - 1];
        builder->SetInsertPoint(current);
        Value *result = last->getResult()->gen();
        if (!returnType)
            returnType = result->getType();
        else if (returnType != result->getType())
            errs() << "mismatched types!\n";
        return nullptr;

        if (!result)
        {
            errs() << "missing result!\n";
            return nullptr;
        }

        builder->CreateBr(merge);
        current = builder->GetInsertBlock();

        blocks.push_back(current);
        results.push_back(result);

        builder->SetInsertPoint(split);
        if (j < exprs.size())
        {
            predicate = getPredFCmp(when);
            if (!predicate)
            {
                errs() << "missing predicate!\n";
                return nullptr;
            }
            current = BasicBlock::Create(*context, "then", parent);
            split = BasicBlock::Create(*context, "else", parent);

            builder->CreateCondBr(predicate, current, split);
        }
        else
        {
            Value *final = chain->getLast()->gen();
            if (!final)
            {
                errs() << "missing result!\n";
                return nullptr;
            }
            builder->CreateBr(merge);
            blocks.push_back(split);
            results.push_back(final);
        }
    }

    parent->insert(parent->end(), merge);
    builder->SetInsertPoint(merge);
    PHINode *phi = builder->CreatePHI(returnType, 2, "iftmp");

    int i = 0;
    for (auto block : blocks)
    {
        phi->addIncoming(results[i++], block);
    }

    return phi;
}

// todo
Value *CodeGen::genBinaryOperation(BinaryOperation *bin)
{
    Value *left = bin->getLeft()->gen();
    Value *right = bin->getRight()->gen();

    if (!left || !right)
    {
        errs() << "error inside binary operator!\n";
        return nullptr;
    }

    switch (bin->getOp())
    {
    case 1:
        return builder->CreateFMul(left, right, "multmp");
    case 2:
        return builder->CreateFDiv(left, right, "divtmp");
    case 3:
        return builder->CreateFRem(left, right, "remtmp");
    case 4:
        return builder->CreateFAdd(left, right, "addtmp");
    case 5:
        return builder->CreateFSub(left, right, "subtmp");

    // boolean operators
    case 6:
        left = builder->CreateFCmpULT(left, right, "ulttmp");
        break;
    case 7:
        left = builder->CreateFCmpUGT(left, right, "ugttmp");
        break;
    case 10:
        left = builder->CreateFCmpULE(left, right, "uletmp");
        break;
    case 11:
        left = builder->CreateFCmpUGE(left, right, "ugetmp");
        break;
    case 12:
        left = builder->CreateFCmpUEQ(left, right, "ueqtmp");
        break;
    case 13:
        left = builder->CreateFCmpUNE(left, right, "unetmp");
        break;
    default:
        errs() << "unknown operator!\n";
        return nullptr;
    }

    // convert int to float
    return builder->CreateUIToFP(left, Type::getDoubleTy(*context), "booltmp");
}

// todo
Value *CodeGen::genWhenExpression(WhenExpression *when)
{
    return ConstantFP::get(*context, APFloat(0.0));
}

// todo
Value *CodeGen::genFunctionCall(FunctionCall *call)
{
    Function *fn = loadFunction(call->getName());
    if (!fn)
    {
        errs() << "unknown function!\n";
        return nullptr;
    }

    int callArgCount = call->getArgs().size();
    if (fn->arg_size() != callArgCount)
    {
        errs() << "mismatched arg count!\n";
        return nullptr;
    }

    vector<Value *> argv;
    for (int i = 0; i < callArgCount; i++)
    {
        argv.push_back(call->getArgs()[i]->gen());
        if (!argv.back())
        {
            errs() << "missing args!\n";
            return nullptr;
        }
    }

    return builder->CreateCall(fn, argv, "calltmp");
}

// todo
Value *CodeGen::genVariableRef(VariableRef *ref)
{
    string name = ref->getName();
    Value *var = namedValues[name];
    if (!var)
    {
        errs() << "unknown variable name!";
        return nullptr;
    }

    // Load the value.
    return builder->CreateLoad(Type::getDoubleTy(*context), var, name);
}