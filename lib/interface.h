#ifndef __INTERFACE_H
#define __INTERFACE_H

#include <iostream>
#include <vector>
#include <memory>

#include "fx.h"
#include "codegen/codegen.h"

using namespace ffi;

vector<unique_ptr<Expr>> translateExpressionVec(const FFISafeExpr *start, uint8_t len);
vector<FFISafeExpr *> translateStringVec(const char *start, uint32_t len);
unique_ptr<WhenExpression> translateWhen(const FFISafeExpr *ptr);
unique_ptr<Expr> translateExpression(const FFISafeExpr *ptr);

vector<unique_ptr<Expr>> reGenerateAST(FFISafeExprVec tokens);

#endif