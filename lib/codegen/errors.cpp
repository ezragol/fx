#include "errors.h"

Location::Location(unsigned int line, unsigned int column, string filename)
    : line(line), column(column), filename(filename){};

string Location::getMessage()
{
    return "@" + filename + ":" + to_string(line + 1) + ":" + to_string(column + 1);
}

CodeGenError::CodeGenError(string message, Location location)
{
    addToStack(message, location);
};

void CodeGenError::addToStack(string message, Location location)
{
    this->stack.push_back(message);
    this->locations.push_back(location);
}

void CodeGenError::print()
{
    llvm::errs() << "\nERROR: [codegen]\n >>   ";
    for (int i = 0; i < stack.size(); i++)
    {
        llvm::errs() << locations.at(i).getMessage() << ", " << stack.at(i) << "\n      ";
    }
    llvm::errs() << "\n";
}

void CodeGenError::printStatic(string message, Location location)
{
    CodeGenError error(message, location);
    error.print();
}