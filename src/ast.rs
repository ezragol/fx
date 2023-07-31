use std::ffi::{c_char, CString};

#[derive(Debug, Clone)]
pub enum Expr {
    // is floating?, value (if int), value (if float)
    NumberLiteral(bool, isize, f64),
    // string literal value
    StringLiteral(String),
    // function name, argument names, function body
    FunctionDefinition(String, Vec<String>, Box<Expr>),
    // chain links -> when expressions as base cases, and finally a recursive expression
    ChainExpression(Vec<Expr>),
    // binary op as unsigned int, left, right
    BinaryOperation(u8, Box<Expr>, Box<Expr>),
    // predicate, result
    WhenExpression(Box<Expr>, Box<Expr>),
    // function name, argument values
    FunctionCall(String, Vec<Expr>),
    // variable name
    VariableRef(String),
}

fn map_vec<T: Clone, U>(from: Vec<T>, f: fn(T) -> U) -> (*const U, usize) {
    let converted: Vec<U> = from.clone().into_iter().map(f).collect();
    let ptr = converted.as_ptr();
    std::mem::forget(converted);
    (ptr, from.len())
}

pub fn convert_vec(from: Vec<Expr>) -> (*const FFISafeExpr, usize) {
    map_vec(from, |e| convert_expr(e))
}

fn convert_str_vec(from: Vec<String>) -> (*const *const c_char, usize) {
    map_vec(from, |s| convert_str(s))
}

fn convert_box(from: Box<Expr>) -> *const FFISafeExpr {
    Box::into_raw(convert_expr(*from).into())
}

fn convert_str(from: String) -> *const c_char {
    let cstr = CString::new(from).unwrap();
    return cstr.into_raw();
}

pub fn convert_expr(expr: Expr) -> FFISafeExpr {
    match expr {
        Expr::NumberLiteral(is_f, int, float) => FFISafeExpr::NumberLiteral(is_f, int, float),
        Expr::StringLiteral(src) => FFISafeExpr::StringLiteral(convert_str(src)),
        Expr::FunctionDefinition(name, args, body) => {
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
    }
}

#[repr(C)]
#[derive(Debug)]
pub enum FFISafeExpr {
    // is floating?, value (if int), value (if float)
    NumberLiteral(bool, isize, f64),
    // string literal value
    StringLiteral(*const c_char),
    // function name, function args start pointer, function args length, function body
    FunctionDefinition(*const c_char, *const *const c_char, usize, *const FFISafeExpr),
    // chain links start pointer, chain length
    ChainExpression(*const FFISafeExpr, usize),
    // binary op as unsigned int, left, right
    BinaryOperation(u8, *const FFISafeExpr, *const FFISafeExpr),
    // predicate, result
    WhenExpression(*const FFISafeExpr, *const FFISafeExpr),
    // function name, argument values start pointer, argument values length
    FunctionCall(*const c_char, *const FFISafeExpr, usize),
    // variable name
    VariableRef(*const c_char),
}

#[repr(C)]
pub struct FFISafeExprVec {
    pub ptr: *const FFISafeExpr,
    pub len: usize,
}
