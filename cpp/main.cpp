#include "compiler.h"

int main(int argc, char *argv[])
{
    char *filename = parse_options(argv, argc);
    dbgs() << "filename: " << filename << "\n";
    int compiled = Compile(filename);
    free(filename);
    return compiled;
}