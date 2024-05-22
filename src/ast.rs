use std::ffi::{c_char, CString};

use crate::errors::Location;

#[repr(C)]
#[derive(Debug)]
pub struct LocatedFFISafeExpr {
    line: usize,
    column: usize,
    filename: *mut c_char,
    expr: FFISafeExpr,
}

impl LocatedFFISafeExpr {
    pub fn new(expr: FFISafeExpr, location: Location) -> LocatedFFISafeExpr {
        LocatedFFISafeExpr {
            line: location.get_line(),
            column: location.get_column(),
            filename: convert_str(location.get_filename()),
            expr,
        }
    }
    
    pub fn get_expr(&self) -> &FFISafeExpr {
        &self.expr
    }

    pub fn get_filename(&self) -> *mut c_char {
        self.filename
    }
}

#[derive(Debug, Clone)]
pub struct LocatedExpr {
    expr: Expr,
    location: Location,
}

impl LocatedExpr {
    pub fn new(expr: Expr, location: Location) -> LocatedExpr {
        LocatedExpr { expr, location }
    }

    pub fn get_location(&self) -> Location {
        self.location
    }

    pub fn get_expr(&self) -> Expr {
        self.expr
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum ReturnType {
    Int,
    Float,
    String
}

#[derive(Debug, Clone)]
pub enum Expr {
    // is floating?, value (if int), value (if float)
    NumberLiteral(bool, isize, f64),
    // string literal value
    StringLiteral(String),
    // function name, argument names, function body, return type
    FunctionDefinition(String, Vec<String>, Box<LocatedExpr>, ReturnType),
    // chain links -> when expressions as base cases, and finally a recursive expression
    ChainExpression(Vec<LocatedExpr>),
    // binary op as unsigned int, left, right
    BinaryOperation(u8, Box<LocatedExpr>, Box<LocatedExpr>),
    // predicate, result
    WhenExpression(Box<LocatedExpr>, Box<LocatedExpr>),
    // function name, argument values
    FunctionCall(String, Vec<LocatedExpr>),
    // variable name
    VariableRef(String),
}

fn map_vec<T: Clone, U>(from: Vec<T>, f: fn(T) -> U) -> (*mut U, usize) {
    let mut converted: Vec<U> = from.clone().into_iter().map(f).collect();
    let ptr = converted.as_mut_ptr();
    std::mem::forget(converted);
    (ptr, from.len())
}

pub fn convert_vec(from: Vec<LocatedExpr>) -> (*mut LocatedFFISafeExpr, usize) {
    map_vec(from, |e| convert_expr(e))
}

fn convert_str_vec(from: Vec<String>) -> (*mut *mut c_char, usize) {
    map_vec(from, |s| convert_str(s))
}

fn convert_box(from: Box<LocatedExpr>) -> *mut LocatedFFISafeExpr {
    Box::into_raw(convert_expr(*from).into())
}

pub fn convert_str(from: String) -> *mut c_char {
    let cstr = CString::new(from).unwrap();
    return cstr.into_raw();
}

pub fn convert_expr(expr: LocatedExpr) -> LocatedFFISafeExpr {
    let located = match expr.expr {
        Expr::NumberLiteral(is_f, int, float) => FFISafeExpr::NumberLiteral(is_f, int, float),
        Expr::StringLiteral(src) => FFISafeExpr::StringLiteral(convert_str(src)),
        Expr::FunctionDefinition(name, args, body, _) => {
            let arg_vec = convert_str_vec(args);
            FFISafeExpr::FunctionDefinition(
                convert_str(name),
                arg_vec.0,
                arg_vec.1,
                convert_box(body),
            )
        }
        Expr::ChainExpression(exprs) => {
            let expr_vec = convert_vec(exprs);
            FFISafeExpr::ChainExpression(expr_vec.0, expr_vec.1)
        }
        Expr::BinaryOperation(op, left, right) => {
            FFISafeExpr::BinaryOperation(op, convert_box(left), convert_box(right))
        }
        Expr::WhenExpression(predicate, result) => {
            FFISafeExpr::WhenExpression(convert_box(predicate), convert_box(result))
        }
        Expr::FunctionCall(name, args) => {
            let arg_vec = convert_vec(args);
            FFISafeExpr::FunctionCall(convert_str(name), arg_vec.0, arg_vec.1)
        }
        Expr::VariableRef(name) => FFISafeExpr::VariableRef(convert_str(name)),
    };
    LocatedFFISafeExpr::new(located, expr.location)
}

#[repr(C)]
#[derive(Debug)]
pub enum FFISafeExpr {
    // is floating?, value (if int), value (if float)
    NumberLiteral(bool, isize, f64),
    // string literal value
    StringLiteral(*mut c_char),
    // function name, function args start pointer, function args length, function body
    FunctionDefinition(
        *mut c_char,
        *mut *mut c_char,
        usize,
        *mut LocatedFFISafeExpr,
    ),
    // chain links start pointer, chain length
    ChainExpression(*mut LocatedFFISafeExpr, usize),
    // binary op as unsigned int, left, right
    BinaryOperation(u8, *mut LocatedFFISafeExpr, *mut LocatedFFISafeExpr),
    // predicate, result
    WhenExpression(*mut LocatedFFISafeExpr, *mut LocatedFFISafeExpr),
    // function name, argument values start pointer, argument values length
    FunctionCall(*mut c_char, *mut LocatedFFISafeExpr, usize),
    // variable name
    VariableRef(*mut c_char),
}

#[repr(C)]
pub struct FFISafeExprVec {
    pub ptr: *mut LocatedFFISafeExpr,
    pub len: usize,
    pub out: *mut c_char,
}
