#ifndef __COMPILER_H
#define __COMPILER_H

#include <string>
#include <stdlib.h>

#include "interface.h"

using namespace std;
using namespace llvm;
using namespace llvm::sys;

int Compile(string OutFile);

#endif