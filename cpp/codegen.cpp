#include "codegen.h"

CodeGen::CodeGen(string TargetTriple, class TargetMachine *TargetMachine)
    : TargetTriple(TargetTriple), TargetMachine(TargetMachine)
{
    TheContext = make_unique<LLVMContext>();
    TheModule = make_unique<Module>("fx", *TheContext);
    Builder = make_unique<IRBuilder<>>(*TheContext);

    TheModule->setTargetTriple(TargetTriple);
    TheModule->setDataLayout(TargetMachine->createDataLayout());
}

int CodeGen::RunPass(string OutFile)
{
    legacy::PassManager Pass;
    auto FileType = CGFT_ObjectFile;
    std::error_code EC;
    raw_fd_ostream Dest(OutFile, EC, sys::fs::OF_None);

    if (EC)
    {
        errs() << "Could not open file: " << EC.message();
        return 1;
    }

    if (TargetMachine->addPassesToEmitFile(Pass, Dest, nullptr, FileType))
    {
        errs() << "TargetMachine can't emit a file of this type";
        return 1;
    }

    TheModule->print(errs(), nullptr);

    Pass.run(*TheModule);
    Dest.flush();

    return 0;
}

// taken from llvm examples (like most things in this)
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
    cout << "can't find function!\n";
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
    cout << "missing return val!\n";
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
        cout << "missing predicate!\n";
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
        cout << "missing predicate!\n";
        return nullptr;
    }

    Function *Parent = Builder->GetInsertBlock()->getParent();
    BasicBlock *Current = BasicBlock::Create(*TheContext, "then", Parent);
    BasicBlock *Split = BasicBlock::Create(*TheContext, "else", Parent);
    BasicBlock *Merge = BasicBlock::Create(*TheContext, "ifcont");

    Builder->CreateCondBr(Predicate, Current, Split);

    const vector<unique_ptr<WhenExpression>> &Exprs = Chain->GetExpressions();
    for (int j = 1; j <= Exprs.size(); j++)
    {
        auto &When = Exprs[j];
        auto &Last = Exprs[j - 1];
        Builder->SetInsertPoint(Current);
        Value *Result = Last->GetResult()->Gen(this);
        if (!Result)
        {
            cout << "missing result!\n";
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
                cout << "missing predicate!\n";
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
                cout << "missing result!\n";
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
        cout << "error inside binary operator!\n";
        return nullptr;
    }

    switch (Bin->GetOp())
    {
    case 1:
        return Builder->CreateFMul(Left, Right, "multmp");
    case 2:
        return Builder->CreateFDiv(Left, Right, "divtmp");
    case 3:
        return Builder->CreateFAdd(Left, Right, "addtmp");
    case 4:
        return Builder->CreateFSub(Left, Right, "subtmp");

    // boolean operators
    case 5:
        Left = Builder->CreateFCmpULT(Left, Right, "ulttmp");
        break;
    case 6:
        Left = Builder->CreateFCmpUGT(Left, Right, "ugttmp");
        break;
    case 9:
        Left = Builder->CreateFCmpULE(Left, Right, "uletmp");
        break;
    case 10:
        Left = Builder->CreateFCmpUGE(Left, Right, "ugetmp");
        break;
    case 11:
        Left = Builder->CreateFCmpUEQ(Left, Right, "ueqtmp");
        break;
    case 12:
        Left = Builder->CreateFCmpUNE(Left, Right, "unetmp");
        break;
    default:
        cout << "unknown operator!\n";
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
        cout << "unknown function!\n";
        return nullptr;
    }

    int CallArgCount = Call->GetArgs().size();
    if (Fn->arg_size() != CallArgCount)
    {
        cout << "mismatched arg count!\n";
        return nullptr;
    }

    vector<Value *> Argv;
    for (int i = 0; i < CallArgCount; i++)
    {
        Argv.push_back(Call->GetArgs()[i]->Gen(this));
        if (!Argv.back())
        {
            cout << "missing args!\n";
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
        cout << "unknown variable name!";
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
    cout << Prefix << "number { " << Floating << ", " << IntVal << ", " << FloatVal << " }\n";
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
    cout << Prefix << "string \"" << StringVal << "\"\n";
}

Value *ast::StringLiteral::Gen(CodeGen *Generator)
{
    return Generator->GenStringLiteral(this);
}

FunctionDefinition::FunctionDefinition(string Name, vector<string> Args, unique_ptr<Expr> Body)
    : Name(Name), Args(Args), Body(move(Body)) {}

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
    cout << Prefix << "fn " << Name << " ( ";
    for (string Arg : Args)
    {
        cout << Arg << " ";
    }
    cout << ") {\n"
         << Prefix;
    Body->Print(Prefix + "  ");
    cout << Prefix << "}\n";
}

Value *FunctionDefinition::Gen(CodeGen *Generator)
{
    return Generator->GenFunctionDefinition(this);
}

ChainExpression::ChainExpression(vector<unique_ptr<WhenExpression>> Expressions, unique_ptr<Expr> Last)
    : Expressions(move(Expressions)), Last(move(Last)) {}

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
    cout << Prefix << "chain { \n";
    for (auto &Expr : Expressions)
    {
        Expr->Print(Prefix + "  ");
    }
    Last->Print(Prefix + "  ");
    cout << Prefix << "}\n";
}

Value *ChainExpression::Gen(CodeGen *Generator)
{
    return Generator->GenChainExpression(this);
}

BinaryOperation::BinaryOperation(uint8_t Op, unique_ptr<Expr> Left, unique_ptr<Expr> Right)
    : Op(Op), Left(move(Left)), Right(move(Right)) {}

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
    cout << Prefix << "op " << unsigned(Op) << " { \n";
    Left->Print(Prefix + "  ");
    Right->Print(Prefix + "  ");
    cout << Prefix << "}\n";
}

Value *BinaryOperation::Gen(CodeGen *Generator)
{
    return Generator->GenBinaryOperation(this);
}

WhenExpression::WhenExpression(unique_ptr<Expr> Predicate, unique_ptr<Expr> Result)
    : Predicate(move(Predicate)), Result(move(Result)) {}

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
    cout << Prefix << "when {\n";
    Predicate->Print(Prefix + "  ");
    Result->Print(Prefix + "  then ");
    cout << Prefix << "}\n";
}

Value *WhenExpression::Gen(CodeGen *Generator)
{
    return Generator->GenWhenExpression(this);
}

FunctionCall::FunctionCall(string Name, vector<unique_ptr<Expr>> Args)
    : Name(Name), Args(move(Args)) {}

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
    cout << Prefix << Name << " (\n";
    for (auto &Arg : Args)
    {
        Arg->Print(Prefix + "  ");
    }
    cout << Prefix << ")\n";
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
    cout << Prefix << "var " << Name << "\n";
}

Value *VariableRef::Gen(CodeGen *Generator)
{
    return Generator->GenVariableRef(this);
}