#ifndef __AST_H
#define __AST_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include "errors.h"

using namespace std;

namespace ast
{
    enum Structure
    {
        BinaryOp,
        ChainExpr,
        FnCall,
        FnDef,
        Num,
        Ref,
        Str,
        WhenExpr
    };

    enum NumberType_en
    {
        i8,
        i16,
        i32,
        i64,
        u8,
        u16,
        u32,
        u64,
        f32,
        f64,
    };

    union NumberType_un
    {
        int8_t i8;
        int16_t i16;
        int32_t i32;
        int64_t i64;
        uint8_t u8;
        uint16_t u16;
        uint32_t u32;
        uint64_t u64;
        float f32;
        double f64;
    };

    struct NumberType
    {
        NumberType_en type;
        NumberType_un value;
    };

    class Expr
    {
        Structure structure;
        Location location;

    public:
        Expr(Structure structure, Location location);

        Structure getStructure();
        Location getLocation();

        virtual ~Expr() = default;
        Expr &operator=(const Expr &) = delete;
    };
}

using namespace ast;

#endif