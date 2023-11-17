#include "ast.h"

Expr::Expr(Structure structure)
    : structure(structure){};

Structure Expr::getStructure()
{
    return structure;
}