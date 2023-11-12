#include "errors.h"

Location::Location(unsigned int line, unsigned int column, string filename)
    : line(line), column(column), filename(filename){};

string Location::GetMessage()
{
    return "@" + filename + " - " + to_string(line) + ":" + to_string(column) + "\n";
}

CodegenError::CodegenError(string message, Location location)
    : message(message), location(location){};

void CodegenError::Print()
{
    llvm::errs() << "[[ error during code generation ]]\n" << location.GetMessage() << message;
}