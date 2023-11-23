#include "errors.h"

Location::Location(unsigned int line, unsigned int column, string filename)
    : line(line), column(column), filename(filename){};

string Location::getMessage()
{
    return "@" + filename + ":" + to_string(line) + ":" + to_string(column);
}

CodegenError::CodegenError(string message, Location location)
{
    addToStack(message, location);
};

void CodegenError::addToStack(string message, Location location)
{
    this->stack.push_back(message);
    this->locations.push_back(location);
}

void CodegenError::print()
{
    llvm::errs() << "[[ error during code generation ]]\n >>";
    for (int i = 0; i < stack.size(); i++)
    {
        llvm::errs() << "   " << locations.at(i).getMessage() << ", " << stack.at(i) << "\n";
    }
}

void CodegenError::printStatic(string message, Location location)
{
    CodegenError error(message, location);
    error.print();
}