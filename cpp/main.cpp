#include "compiler.h"

int main(int argc, char *argv[])
{
    char *filename = parse_options(argv, argc);
    cout << "filename: " << filename << "\n" << std::endl;
    // return Compile("./test/test.bc");
}