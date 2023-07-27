#ifndef __INTERFACE_H
#define __INTERFACE_H

#include <iostream>
#include <vector>
#include <memory>

#include "fx.h"
#include "codegen.h"

using namespace ffi;

vector<FFISafeExpr*> TranslateRustVec(FFISafeExprVec rustVec);
vector<FFISafeExpr*> TranslateStringVec(const char *ptr, uint32_t len);
unique_ptr<Expr> TranslateExpression(const FFISafeExpr *Raw);
vector<unique_ptr<Expr>> ReGenerateAST(FFISafeExprVec Tokens);
void PrintAST(vector<unique_ptr<Expr>> &Tree);

#endif