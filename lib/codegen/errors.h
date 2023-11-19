#include <iostream>

#include <llvm/Support/raw_ostream.h>

using namespace std;

class Location
{
private:
    unsigned int line;
    unsigned int column;
    string filename;

public:
    Location(unsigned int line, unsigned int column, string filename);
    string getMessage();
};

class CodegenError
{
private:
    string message;
    Location location;

public:
    CodegenError(string message, Location location);
    void print();
};