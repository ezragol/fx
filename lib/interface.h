#ifndef __INTERFACE_H
#define __INTERFACE_H

#include <iostream>
#include <vector>
#include <memory>

#include "fx.h"
#include "codegen/codegen.h"

using namespace ffi;

Location buildFromPtr(const LocatedFFISafeExpr *ptr);

vector<unique_ptr<Expr>> translateExpressionVec(const LocatedFFISafeExpr *start, uint8_t len);
vector<FFISafeExpr *> translateStringVec(const char *start, uint32_t len);
unique_ptr<WhenExpression> translateWhen(const LocatedFFISafeExpr *ptr);
unique_ptr<Expr> translateExpression(const LocatedFFISafeExpr *ptr);

vector<unique_ptr<Expr>> reGenerateAST(FFISafeExprVec tokens);

#endif