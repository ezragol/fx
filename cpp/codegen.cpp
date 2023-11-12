#include "codegen.h"

CodeGen::CodeGen(string TargetTriple, TargetMachine *TheTargetMachine)
    : TargetTriple(TargetTriple), TheTargetMachine(TheTargetMachine)
{
    TheContext = make_unique<LLVMContext>();
    TheModule = make_unique<Module>("fx", *TheContext);
    Builder = make_unique<IRBuilder<>>(*TheContext);

    TheModule->setTargetTriple(TargetTriple);
    TheModule->setDataLayout(TheTargetMachine->createDataLayout());
}

int CodeGen::RunPass(string OutFile)
{
    auto FileType = CGFT_ObjectFile;
    std::error_code EC;
    raw_fd_ostream Dest(OutFile, EC, sys::fs::OF_None);

    if (EC)
    {
        errs() << "Could not open file: " << EC.message();
        return 1;
    }

    LoopAnalysisManager LAM;
    FunctionAnalysisManager FAM;
    CGSCCAnalysisManager CGAM;
    ModuleAnalysisManager MAM;
    PassBuilder PB(TheTargetMachine);

    PB.registerModuleAnalyses(MAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

    ModulePassManager MPM = PB.buildPerModuleDefaultPipeline(OptimizationLevel::O2);
    MPM.run(*TheModule, MAM);

    WriteBitcodeToFile(*TheModule, Dest);
    Dest.flush();

    // TheModule->print(dbgs(), nullptr);

    return 0;
}

// taken from llvm examples (like most things)
AllocaInst *CodeGen::CreateEntryBlockAlloca(Function *TheFunction, StringRef VarName)
{
    IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                     TheFunction->getEntryBlock().begin());
    return TmpB.CreateAlloca(Type::getDoubleTy(*TheContext), nullptr, VarName);
}

Function *CodeGen::LoadFunction(string Name)
{
    if (auto *Fn = TheModule->getFunction(Name))
    {
        return Fn;
    }
    errs() << "can't find function!\n";
    return nullptr;
}

Value *CodeGen::GenNumberLiteral(NumberLiteral *Num)
{
    if (Num->IsFloating())
    {
        return ConstantFP::get(*TheContext, APFloat(Num->GetFloatVal()));
    }
    else
    {
        const int &intVal = Num->GetIntVal();
        uint8_t bits = floor(log2(intVal)) + 1;
        return ConstantInt::get(*TheContext, APInt(bits, intVal));
    }
}

Function *CodeGen::GenFunctionDefinition(FunctionDefinition *Def)
{
    // define argument types and return type
    vector<Type *> ArgT(Def->GetArgs().size(), Type::getDoubleTy(*TheContext));
    FunctionType *FuncT = FunctionType::get(Type::getDoubleTy(*TheContext), ArgT, false);
    Function *Fn = Function::Create(FuncT, Function::ExternalLinkage, Def->GetName(), TheModule.get());
    BasicBlock *Block = BasicBlock::Create(*TheContext, "entry", Fn);
    Builder->SetInsertPoint(Block);
    NamedValues.clear();

    // create a "named value" (aka variable) for each argument of the function
    unsigned int i = 0;
    for (auto &Arg : Fn->args())
    {
        string Name = Def->GetArgs()[i++];
        Arg.setName(Name);
        AllocaInst *Alloca = CreateEntryBlockAlloca(Fn, Name);
        Builder->CreateStore(&Arg, Alloca);
        NamedValues[Name] = Alloca;
    }

    if (Value *Returned = Def->GetBody()->Gen(this))
    {
        Builder->CreateRet(Returned);
        verifyFunction(*Fn);
        return Fn;
    }

    Fn->eraseFromParent();
    errs() << "missing return val!\n";
    return nullptr;
}

// todo
Value *CodeGen::GenStringLiteral(ast::StringLiteral *String)
{
    return ConstantFP::get(*TheContext, APFloat(0.0));
}

Value *CodeGen::GetPredFCmp(const unique_ptr<WhenExpression> &When)
{
    Value *Predicate = When->GetPredicate()->Gen(this);
    if (!Predicate)
    {
        errs() << "missing predicate!\n";
        return nullptr;
    }
    return Builder->CreateFCmpONE(Predicate, ConstantFP::get(*TheContext, APFloat(0.0)), "ifcond");
}

// todo
Value *CodeGen::GenChainExpression(ChainExpression *Chain)
{
    vector<BasicBlock *> Blocks;
    vector<Value *> Results;

    Value *Predicate = GetPredFCmp(Chain->GetExpressions()[0]);
    if (!Predicate)
    {
        errs() << "missing predicate!\n";
        return nullptr;
    }

    Function *Parent = Builder->GetInsertBlock()->getParent();
    BasicBlock *Current = BasicBlock::Create(*TheContext, "then", Parent);
    BasicBlock *Split = BasicBlock::Create(*TheContext, "else", Parent);
    BasicBlock *Merge = BasicBlock::Create(*TheContext, "ifcont");
    Type *ReturnType = nullptr;

    Builder->CreateCondBr(Predicate, Current, Split);

    const vector<unique_ptr<WhenExpression>> &Exprs = Chain->GetExpressions();
    for (int j = 1; j <= Exprs.size(); j++)
    {
        auto &When = Exprs[j];
        auto &Last = Exprs[j - 1];
        Builder->SetInsertPoint(Current);
        Value *Result = Last->GetResult()->Gen(this);
        if (!ReturnType)
            ReturnType = Result->getType();
        else if (ReturnType != Result->getType())

        if (!Result)
        {
            errs() << "missing result!\n";
            return nullptr;
        }

        Builder->CreateBr(Merge);
        Current = Builder->GetInsertBlock();

        Blocks.push_back(Current);
        Results.push_back(Result);

        Builder->SetInsertPoint(Split);
        if (j < Exprs.size())
        {
            Predicate = GetPredFCmp(When);
            if (!Predicate)
            {
                errs() << "missing predicate!\n";
                return nullptr;
            }
            Current = BasicBlock::Create(*TheContext, "then", Parent);
            Split = BasicBlock::Create(*TheContext, "else", Parent);

            Builder->CreateCondBr(Predicate, Current, Split);
        }
        else
        {
            Value *Final = Chain->GetLast()->Gen(this);
            if (!Final)
            {
                errs() << "missing result!\n";
                return nullptr;
            }
            Builder->CreateBr(Merge);
            Blocks.push_back(Split);
            Results.push_back(Final);
        }
    }

    Parent->insert(Parent->end(), Merge);
    Builder->SetInsertPoint(Merge);
    PHINode *Phi = Builder->CreatePHI(Type::getDoubleTy(*TheContext), 2, "iftmp");

    int i = 0;
    for (auto Block : Blocks)
    {
        Phi->addIncoming(Results[i++], Block);
    }

    return Phi;
}

// todo
Value *CodeGen::GenBinaryOperation(BinaryOperation *Bin)
{
    Value *Left = Bin->GetLeft()->Gen(this);
    Value *Right = Bin->GetRight()->Gen(this);

    if (!Left || !Right)
    {
        errs() << "error inside binary operator!\n";
        return nullptr;
    }

    switch (Bin->GetOp())
    {
    case 1:
        return Builder->CreateFMul(Left, Right, "multmp");
    case 2:
        return Builder->CreateFDiv(Left, Right, "divtmp");
    case 3:
        return Builder->CreateFRem(Left, Right, "remtmp");
    case 4:
        return Builder->CreateFAdd(Left, Right, "addtmp");
    case 5:
        return Builder->CreateFSub(Left, Right, "subtmp");

    // boolean operators
    case 6:
        Left = Builder->CreateFCmpULT(Left, Right, "ulttmp");
        break;
    case 7:
        Left = Builder->CreateFCmpUGT(Left, Right, "ugttmp");
        break;
    case 10:
        Left = Builder->CreateFCmpULE(Left, Right, "uletmp");
        break;
    case 11:
        Left = Builder->CreateFCmpUGE(Left, Right, "ugetmp");
        break;
    case 12:
        Left = Builder->CreateFCmpUEQ(Left, Right, "ueqtmp");
        break;
    case 13:
        Left = Builder->CreateFCmpUNE(Left, Right, "unetmp");
        break;
    default:
        errs() << "unknown operator!\n";
        return nullptr;
    }

    // convert int to float
    return Builder->CreateUIToFP(Left, Type::getDoubleTy(*TheContext), "booltmp");
}

// todo
Value *CodeGen::GenWhenExpression(WhenExpression *When)
{
    return ConstantFP::get(*TheContext, APFloat(0.0));
}

// todo
Value *CodeGen::GenFunctionCall(FunctionCall *Call)
{
    Function *Fn = LoadFunction(Call->GetName());
    if (!Fn)
    {
        errs() << "unknown function!\n";
        return nullptr;
    }

    int CallArgCount = Call->GetArgs().size();
    if (Fn->arg_size() != CallArgCount)
    {
        errs() << "mismatched arg count!\n";
        return nullptr;
    }

    vector<Value *> Argv;
    for (int i = 0; i < CallArgCount; i++)
    {
        Argv.push_back(Call->GetArgs()[i]->Gen(this));
        if (!Argv.back())
        {
            errs() << "missing args!\n";
            return nullptr;
        }
    }

    return Builder->CreateCall(Fn, Argv, "calltmp");
}

// todo
Value *CodeGen::GenVariableRef(VariableRef *Ref)
{
    string Name = Ref->GetName();
    Value *V = NamedValues[Name];
    if (!V)
    {
        errs() << "unknown variable name!";
        return nullptr;
    }

    // Load the value.
    return Builder->CreateLoad(Type::getDoubleTy(*TheContext), V, Name);
}

/////////

NumberLiteral::NumberLiteral(bool Floating, int IntVal, double FloatVal)
    : Floating(Floating), IntVal(IntVal), FloatVal(FloatVal) {}

const bool &NumberLiteral::IsFloating()
{
    return Floating;
}

const int &NumberLiteral::GetIntVal()
{
    return IntVal;
}

const double &NumberLiteral::GetFloatVal()
{
    return FloatVal;
}

void NumberLiteral::Print(string Prefix)
{
    dbgs() << Prefix << "number { " << Floating << ", " << IntVal << ", " << FloatVal << " }\n";
}

Value *NumberLiteral::Gen(CodeGen *Generator)
{
    return Generator->GenNumberLiteral(this);
}

ast::StringLiteral::StringLiteral(string StringVal)
    : StringVal(StringVal) {}

const string &ast::StringLiteral::GetStringVal()
{
    return StringVal;
}

void ast::StringLiteral::Print(string Prefix)
{
    dbgs() << Prefix << "string \"" << StringVal << "\"\n";
}

Value *ast::StringLiteral::Gen(CodeGen *Generator)
{
    return Generator->GenStringLiteral(this);
}

FunctionDefinition::FunctionDefinition(string Name, vector<string> Args, unique_ptr<Expr> Body)
    : Name(Name), Args(Args), Body(std::move(Body)) {}

const string &FunctionDefinition::GetName()
{
    return Name;
}

const vector<string> &FunctionDefinition::GetArgs()
{
    return Args;
}

const unique_ptr<Expr> &FunctionDefinition::GetBody()
{
    return Body;
}

void FunctionDefinition::Print(string Prefix)
{
    dbgs() << Prefix << "fn " << Name << " ( ";
    for (string Arg : Args)
    {
        dbgs() << Arg << " ";
    }
    dbgs() << ") {\n"
         << Prefix;
    Body->Print(Prefix + "  ");
    dbgs() << Prefix << "}\n";
}

Value *FunctionDefinition::Gen(CodeGen *Generator)
{
    return Generator->GenFunctionDefinition(this);
}

ChainExpression::ChainExpression(vector<unique_ptr<WhenExpression>> Expressions, unique_ptr<Expr> Last)
    : Expressions(std::move(Expressions)), Last(std::move(Last)) {}

const vector<unique_ptr<WhenExpression>> &ChainExpression::GetExpressions()
{
    return Expressions;
}

const unique_ptr<Expr> &ChainExpression::GetLast()
{
    return Last;
}

void ChainExpression::Print(string Prefix)
{
    dbgs() << Prefix << "chain { \n";
    for (auto &Expr : Expressions)
    {
        Expr->Print(Prefix + "  ");
    }
    Last->Print(Prefix + "  ");
    dbgs() << Prefix << "}\n";
}

Value *ChainExpression::Gen(CodeGen *Generator)
{
    return Generator->GenChainExpression(this);
}

BinaryOperation::BinaryOperation(uint8_t Op, unique_ptr<Expr> Left, unique_ptr<Expr> Right)
    : Op(Op), Left(std::move(Left)), Right(std::move(Right)) {}

const uint8_t &BinaryOperation::GetOp()
{
    return Op;
}

const unique_ptr<Expr> &BinaryOperation::GetLeft()
{
    return Left;
}

const unique_ptr<Expr> &BinaryOperation::GetRight()
{
    return Right;
}

void BinaryOperation::Print(string Prefix)
{
    dbgs() << Prefix << "op " << unsigned(Op) << " { \n";
    Left->Print(Prefix + "  ");
    Right->Print(Prefix + "  ");
    dbgs() << Prefix << "}\n";
}

Value *BinaryOperation::Gen(CodeGen *Generator)
{
    return Generator->GenBinaryOperation(this);
}

WhenExpression::WhenExpression(unique_ptr<Expr> Predicate, unique_ptr<Expr> Result)
    : Predicate(std::move(Predicate)), Result(std::move(Result)) {}

const unique_ptr<Expr> &WhenExpression::GetPredicate()
{
    return Predicate;
}

const unique_ptr<Expr> &WhenExpression::GetResult()
{
    return Result;
}
void WhenExpression::Print(string Prefix)
{
    dbgs() << Prefix << "when {\n";
    Predicate->Print(Prefix + "  ");
    Result->Print(Prefix + "  then ");
    dbgs() << Prefix << "}\n";
}

Value *WhenExpression::Gen(CodeGen *Generator)
{
    return Generator->GenWhenExpression(this);
}

FunctionCall::FunctionCall(string Name, vector<unique_ptr<Expr>> Args)
    : Name(Name), Args(std::move(Args)) {}

const string &FunctionCall::GetName()
{
    return Name;
}

const vector<unique_ptr<Expr>> &FunctionCall::GetArgs()
{
    return Args;
}

void FunctionCall::Print(string Prefix)
{
    dbgs() << Prefix << Name << " (\n";
    for (auto &Arg : Args)
    {
        Arg->Print(Prefix + "  ");
    }
    dbgs() << Prefix << ")\n";
}

Value *FunctionCall::Gen(CodeGen *Generator)
{
    return Generator->GenFunctionCall(this);
}

VariableRef::VariableRef(string Name)
    : Name(Name) {}

const string &VariableRef::GetName()
{
    return Name;
}

void VariableRef::Print(string Prefix)
{
    dbgs() << Prefix << "var " << Name << "\n";
}

Value *VariableRef::Gen(CodeGen *Generator)
{
    return Generator->GenVariableRef(this);
}