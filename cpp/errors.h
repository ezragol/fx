#include <iostream>

#include <llvm/Support/raw_ostream.h>

using namespace std;

class ErrorLocation {
private:
    unsigned int Line;
    unsigned int Column;
    string Filename;
public:
    Location(unsigned int Line, unsigned int Column, string Filename);
    string GetMessage();
};

class CodegenError {
private:
    string Message;
    ErrorLocation Location;
public:
    CodegenError(string message, ErrorLocation Location);
    void Print();
};