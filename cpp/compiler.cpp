#include "compiler.h"

// all taken from llvm examples
int Compile(string OutFile)
{
    auto TargetTriple = sys::getDefaultTargetTriple();
    InitializeNativeTarget();
    InitializeNativeTargetAsmParser();
    string Error;
    auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);

    if (!Target)
    {
        errs() << Error;
        return 1;
    }
    auto CPU = "generic";
    auto Features = "";

    TargetOptions Opt;
    auto RM = optional<Reloc::Model>();
    auto TargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, Opt, RM);
    CodeGen Generator(TargetTriple, TargetMachine);

    FFISafeExprVec Tokens = recieve_tokens();
    auto Tree = ReGenerateAST(Tokens);
    PrintAST(Tree);
    // for (auto &branch : Tree)
    // {
    //     branch->Gen(&Generator);
    // }

    // if (Generator.RunPass(OutFile))
    //     return 1;

    return 0;
}