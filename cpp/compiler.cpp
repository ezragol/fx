// all taken from llvm examples
int Compile(string OutFile)
{
    auto TargetTriple = sys::getDefaultTargetTriple();
    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();
    std::string Error;
    auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);

    if (!Target)
    {
        errs() << Error;
        return 1;
    }
    auto CPU = "generic";
    auto Features = "";

    TargetOptions opt;
    auto RM = Optional<Reloc::Model>();
    auto TargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);
    CodeGen Generator(TargetTriple, TargetMachine);

    std::error_code EC;
    raw_fd_ostream dest(OutFile, EC, sys::fs::OF_None);

    if (EC)
    {
        errs() << "Could not open file: " << EC.message();
        return 1;
    }

    if (Generator.RunPass())
        return 1;

    FFISafeExprVec Tokens = recieve_tokens();
    auto Tree = ReGenerateAST(Tokens);
    for (auto &branch : Tree)
    {
        branch->Gen(&Generator);
    }
}