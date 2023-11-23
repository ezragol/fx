#include "codegen.h"

CodeGen::CodeGen(string targetTriple, TargetMachine *targetMachine)
    : targetTriple(targetTriple), targetMachine(targetMachine), error(nullptr)
{
    context = make_unique<LLVMContext>();
    llvmModule = make_unique<Module>("fx", *context);
    builder = make_unique<IRBuilder<>>(*context);

    llvmModule->setTargetTriple(targetTriple);
    llvmModule->setDataLayout(targetMachine->createDataLayout());
}

CodeGen::~CodeGen()
{
    delete error;
}

const unique_ptr<LLVMContext> &CodeGen::getContext()
{
    return context;
}

void CodeGen::addToError(string message, Location location)
{
    if (!error)
    {
        error = new CodeGenError(message, location);
    }
    else
    {
        error->addToStack(message, location);
    }
}

void CodeGen::printError()
{
    error->print();
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
    addToError("unknown function '" + name + "'", internal_err);
    return nullptr;
}

Value *CodeGen::genericGen(const unique_ptr<Expr> &expr)
{
    switch (expr->getStructure())
    {
    case BinaryOp:
        return genBinaryOperation(derived(BinaryOperation) expr);
    case ChainExpr:
        return genChainExpression(derived(ChainExpression) expr);
    case FnCall:
        return genFunctionCall(derived(FunctionCall) expr);
    case FnDef:
        return genFunctionDefinition(derived(FunctionDefinition) expr);
    case Num:
        return genNumberLiteral(derived(NumberLiteral) expr);
    case Ref:
        return genVariableRef(derived(VariableRef) expr);
    case Str:
        return genStringLiteral(derived(ast::StringLiteral) expr);
    case WhenExpr:
        addToError("dangling 'when' not allowed", expr->getLocation());
        break;
    default:
        addToError("unknown generator action", expr->getLocation());
        break;
    }
    return nullptr;
}

Function *CodeGen::genFunctionDefinition(const unique_ptr<FunctionDefinition> &def)
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

    if (Value *returned = genericGen(def->getBody()))
    {
        builder->CreateRet(returned);
        verifyFunction(*fn);
        return fn;
    }

    fn->eraseFromParent();
    addToError("while parsing function definition", def->getLocation());
    return nullptr;
}

Value *CodeGen::genNumberLiteral(const unique_ptr<NumberLiteral> &num)
{
    auto data = num->getValue();
    switch (data.type)
    {
    case i64:
        return ConstantInt::get(*context, APInt(64, data.value.i64, true));
    case u64:
        return ConstantInt::get(*context, APInt(64, data.value.u64));
    case f64:
        return ConstantFP::get(*context, APFloat(data.value.f64));
    default:
        addToError("unknown number type", num->getLocation());
        return nullptr;
    }
}

// todo
Value *CodeGen::genStringLiteral(const unique_ptr<ast::StringLiteral> &string)
{
    return ConstantFP::get(*context, APFloat(0.0));
}

Value *CodeGen::getPredFCmp(const unique_ptr<WhenExpression> &when)
{
    Value *predicate = genericGen(when->getPredicate());
    if (!predicate)
    {
        addToError("while parsing predicate!\n", when->getLocation());
        return nullptr;
    }
    return builder->CreateFCmpONE(predicate, ConstantFP::get(*context, APFloat(0.0)), "ifcond");
}

// todo
Value *CodeGen::genChainExpression(const unique_ptr<ChainExpression> &chain)
{
    vector<BasicBlock *> blocks;
    vector<Value *> results;

    Value *predicate = getPredFCmp(chain->getExpressions()[0]);
    if (!predicate)
    {
        addToError("while parsing chain", chain->getLocation());
        return nullptr;
    }

    Function *parent = builder->GetInsertBlock()->getParent();
    BasicBlock *current = BasicBlock::Create(*context, "then", parent);
    BasicBlock *split = BasicBlock::Create(*context, "else", parent);
    BasicBlock *merge = BasicBlock::Create(*context, "ifcont");

    builder->CreateCondBr(predicate, current, split);

    const vector<unique_ptr<WhenExpression>> &exprs = chain->getExpressions();
    for (int j = 1; j <= exprs.size(); j++)
    {
        auto &when = exprs[j];
        auto &last = exprs[j - 1];
        builder->SetInsertPoint(current);
        Value *result = genericGen(last->getResult());

        if (!result)
        {
            addToError("while parsing chain link", last->getLocation());
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
                addToError("while parsing chain link", when->getLocation());
                return nullptr;
            }
            current = BasicBlock::Create(*context, "then", parent);
            split = BasicBlock::Create(*context, "else", parent);

            builder->CreateCondBr(predicate, current, split);
        }
        else
        {
            Value *final = genericGen(chain->getLast());
            if (!final)
            {
                addToError("while parsing last chain link", chain->getLast()->getLocation());
                return nullptr;
            }
            builder->CreateBr(merge);
            blocks.push_back(split);
            results.push_back(final);
        }
    }

    parent->insert(parent->end(), merge);
    builder->SetInsertPoint(merge);
    PHINode *phi = builder->CreatePHI(Type::getDoubleTy(*context), 2, "iftmp");

    int i = 0;
    for (auto block : blocks)
    {
        phi->addIncoming(results[i++], block);
    }

    return phi;
}

// todo
Value *CodeGen::genBinaryOperation(const unique_ptr<BinaryOperation> &bin)
{
    Value *left = genericGen(bin->getLeft());
    Value *right = genericGen(bin->getRight());

    if (!left)
    {
        addToError("while parsing left side of binary operation", bin->getLeft()->getLocation());
        return nullptr;
    }

    if (!right)
    {
        addToError("while parsing right side of binary operation", bin->getRight()->getLocation());
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
        addToError("unknown operator", bin->getLocation());
        return nullptr;
    }

    // convert int to float
    return builder->CreateUIToFP(left, Type::getDoubleTy(*context), "booltmp");
}

// todo
Value *CodeGen::genFunctionCall(const unique_ptr<FunctionCall> &call)
{
    Function *fn = loadFunction(call->getName());
    if (!fn)
    {
        addToError("while loading function '" + call->getName() + "'", call->getLocation());
        return nullptr;
    }

    int callArgCount = call->getArgs().size();
    if (fn->arg_size() != callArgCount)
    {
        addToError("mismatched argument count ", call->getLocation());
        return nullptr;
    }

    vector<Value *> argv;
    for (int i = 0; i < callArgCount; i++)
    {
        argv.push_back(genericGen(call->getArgs()[i]));
        if (!argv.back())
        {
            addToError("broken arguments", call->getLocation());
            return nullptr;
        }
    }

    return builder->CreateCall(fn, argv, "calltmp");
}

// todo
Value *CodeGen::genVariableRef(const unique_ptr<VariableRef> &ref)
{
    string name = ref->getName();
    Value *var = namedValues[name];
    if (!var)
    {
        addToError("unknown variable", ref->getLocation());
        return nullptr;
    }

    // Load the value.
    return builder->CreateLoad(Type::getDoubleTy(*context), var, name);
}