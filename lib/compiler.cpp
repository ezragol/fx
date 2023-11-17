#include "compiler.h"

// all taken from llvm examples
int compile(int argc, char *argv[])
{
    auto targetTriple = sys::getDefaultTargetTriple();
    InitializeNativeTarget();
    InitializeNativeTargetAsmParser();
    InitializeNativeTargetAsmPrinter();
    string error;
    auto target = TargetRegistry::lookupTarget(targetTriple, error);

    if (!target)
    {
        errs() << error;
        return 1;
    }
    auto cpu = "generic";
    auto features = "";

    TargetOptions opt;
    auto rm = optional<Reloc::Model>();
    auto targetMachine = target->createTargetMachine(targetTriple, cpu, features, opt, rm);
    CodeGen generator(targetTriple, targetMachine);
    FFISafeExprVec tokens = recieve_tokens(argv, argc);
    auto tree = reGenerateAST(tokens);
    drop_all(tokens.ptr, tokens.len);

    for (auto &branch : tree)
    {
        generator.genericGen(branch);
    }

    if (generator.runPass(tokens.out))
        return 1;

    delete targetMachine;
    return 0;
}