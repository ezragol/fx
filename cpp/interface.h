#ifndef __INTERFACE_H
#define __INTERFACE_H

#include <iostream>
#include <vector>
#include <memory>

#include "fx.h"
#include "codegen.h"

using namespace ffi;

vector<unique_ptr<Expr>> TranslateExpressionVec(const FFISafeExpr *Start, uint8_t Len);
vector<FFISafeExpr *> TranslateStringVec(const char *Start, uint32_t Len);
unique_ptr<WhenExpression> TranslateWhen(const FFISafeExpr *Ptr);
unique_ptr<Expr> TranslateExpression(const FFISafeExpr *Ptr);

vector<unique_ptr<Expr>> ReGenerateAST(FFISafeExprVec Tokens);
void PrintAST(vector<unique_ptr<Expr>> &Tree);

#endif