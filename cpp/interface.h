#ifndef __INTERFACE_H
#define __INTERFACE_H

#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdint.h>
#include <sstream>

#include "fx.h"

using namespace std;
using namespace ffi;

vector<FFISafeExpr*> TranslateRustVec(FFISafeExprVec rustVec);
vector<FFISafeExpr*> TranslateVecPtr(FFISafeExpr* ptr, uint32_t len, uint32_t capacity);
vector<FFISafeExpr*> TranslateStringVec(const char *ptr, uint32_t len, uint32_t capacity);

#endif