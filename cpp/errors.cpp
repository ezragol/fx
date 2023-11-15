#include "errors.h"

Location::Location(unsigned int Line, unsigned int Column, string Filename)
    : Line(Line), Column(Column), Filename(Filename){};

string Location::GetMessage()
{
    return "@" + Filename + " - " + to_string(Line) + ":" + to_string(Column) + "\n";
}

CodegenError::CodegenError(string Message, ErrorLocation Location)
    : Message(Message), Location(Location){};

void CodegenError::Print()
{
    llvm::errs() << "[[ error during code generation ]]\n"
                 << Location.GetMessage() << Message;
}