#ifndef __AST_H
#define __AST_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>

using namespace std;

namespace ast
{
    enum Structure {
        BinaryOp,
        ChainExpr,
        FnCall,
        FnDef,
        Num,
        Ref,
        Str,
        WhenExpr
    };

    class Expr
    {
        Structure structure;
    public:
        Expr(Structure structure);

        Structure getStructure();

        virtual ~Expr() = default;
        Expr &operator=(const Expr &) = delete;
    };
}

using namespace ast;

#endif