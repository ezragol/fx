#include <iostream>

#include <llvm/Support/raw_ostream.h>

using namespace std;

#define internal_err Location(0, 0, "internal")
#define err(M, L) CodegenError::printStatic(M, L) 

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
    vector<string> stack;
    vector<Location> locations;

public:
    CodegenError(string message, Location location);
    
    void addToStack(string message, Location location);
    void print();

    static void printStatic(string message, Location location);
};