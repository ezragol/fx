#include "ast.h"

Expr::Expr(Structure structure, Location location)
    : structure(structure), location(location) {};

Structure Expr::getStructure()
{
    return structure;
}

Location Expr::getLocation()
{
    return location;
}