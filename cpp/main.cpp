#include "compiler.h"

int main(int argc, char *argv[])
{
    char *filename = parse_options(argv, argc);
    errs() << "filename: " << filename << "\n";
    free(filename);
    // return Compile("./test/test.bc");
}