// pub trait Expression: std::fmt::Debug {}

use std::ffi::{CString, c_char};

#[derive(Debug, Clone)]
pub enum Expr {
    NumberLiteral(bool, isize, f64),
    StringLiteral(String),
    VariableDefinition(String, Box<Expr>),
    FunctionDefinition(String, Vec<String>, Box<Expr>),
    ChainExpression(Vec<Expr>),
    BinaryOperation(u8, Box<Expr>, Box<Expr>),
    WhenExpression(Box<Expr>, Box<Expr>),
    FunctionCall(String, Vec<Expr>),
    VariableRef(String),
}

fn map_vec<T: Clone, U>(from: Vec<T>, f: fn(T) -> U) -> (*mut U, usize, usize) {
    let mut converted: Vec<U> = from.clone().into_iter().map(f).collect();
    let ptr = converted.as_mut_ptr();
    std::mem::forget(converted);
    (ptr, from.len(), from.capacity())
}

pub fn convert_vec(from: Vec<Expr>) -> (*mut FFISafeExpr, usize, usize) {
    map_vec(from, |e| convert_expr(e))
}

fn convert_str_vec(from: Vec<String>) -> (*mut *const c_char, usize, usize) {
    map_vec(from, |s| convert_str(s))
}

fn convert_box(from: Box<Expr>) -> *mut FFISafeExpr {
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
        Expr::VariableDefinition(name, value) => {
            FFISafeExpr::VariableDefinition(convert_str(name), convert_box(value))
        }
        Expr::FunctionDefinition(name, args, body) => {
            let arg_vec = convert_str_vec(args);
            FFISafeExpr::FunctionDefinition(
                convert_str(name),
                arg_vec.0,
                arg_vec.1,
                arg_vec.2,
                convert_box(body),
            )
        }
        Expr::ChainExpression(exprs) => {
            let expr_vec = convert_vec(exprs);
            FFISafeExpr::ChainExpression(expr_vec.0, expr_vec.1, expr_vec.2)
        }
        Expr::BinaryOperation(op, left, right) => {
            FFISafeExpr::BinaryOperation(op, convert_box(left), convert_box(right))
        }
        Expr::WhenExpression(result, predicate) => {
            FFISafeExpr::WhenExpression(convert_box(result), convert_box(predicate))
        }
        Expr::FunctionCall(name, args) => {
            let arg_vec = convert_vec(args);
            FFISafeExpr::FunctionCall(convert_str(name), arg_vec.0, arg_vec.1, arg_vec.2)
        }
        Expr::VariableRef(name) => FFISafeExpr::VariableRef(convert_str(name)),
    }
}

#[repr(C)]
#[derive(Debug)]
pub enum FFISafeExpr {
    NumberLiteral(bool, isize, f64),
    StringLiteral(*const c_char),
    VariableDefinition(*const c_char, *mut FFISafeExpr),
    FunctionDefinition(
        *const c_char,
        *mut *const c_char,
        usize,
        usize,
        *mut FFISafeExpr,
    ),
    ChainExpression(*mut FFISafeExpr, usize, usize),
    BinaryOperation(u8, *mut FFISafeExpr, *mut FFISafeExpr),
    WhenExpression(*mut FFISafeExpr, *mut FFISafeExpr),
    FunctionCall(*const c_char, *mut FFISafeExpr, usize, usize),
    VariableRef(*const c_char),
}

#[repr(C)]
pub struct FFISafeExprVec {
    pub ptr: *mut FFISafeExpr,
    pub len: usize,
    pub capacity: usize,
}
