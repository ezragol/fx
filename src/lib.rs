use ast::*;
use options::Options;
use parser::Parser;
use std::ffi::c_char;

mod ast;
mod errors;
mod lexer;
mod options;
mod parser;

#[no_mangle]
pub extern "C" fn recieve_tokens(start: *mut *mut c_char, size: usize) -> FFISafeExprVec {
    let options = Options::new(start, size).expect("no filename given!");
    let tree = Parser::new(&options.filename).expect("missing file!").run();
    
    let ffi_safe_tree = convert_vec(tree);
    let ffi_safe = FFISafeExprVec {
        ptr: ffi_safe_tree.0,
        len: ffi_safe_tree.1,
        out: convert_str(options.outfile),
    };
    return ffi_safe;
}

unsafe fn box_drop<T>(ffi_val: *mut T) {
    drop(Box::from_raw(ffi_val));
}

unsafe fn drop_expr(expr: &LocatedFFISafeExpr) {
    match *expr.get_expr() {
        FFISafeExpr::StringLiteral(s) => {
            box_drop(s);
        }
        FFISafeExpr::FunctionDefinition(name, arg_start, len, body) => {
            let args = std::slice::from_raw_parts(arg_start, len);
            box_drop(name);
            for &arg in args {
                box_drop(arg);
            }
            if len > 0 {
                box_drop(arg_start);
            }
            drop_expr(body.as_ref().unwrap());
            box_drop(body);
        }
        FFISafeExpr::ChainExpression(chain_start, len) => {
            drop_all(chain_start, len);
        }
        FFISafeExpr::BinaryOperation(_, left, right) | FFISafeExpr::WhenExpression(left, right) => {
            drop_expr(left.as_ref().unwrap());
            drop_expr(right.as_ref().unwrap());
            box_drop(left);
            box_drop(right);
        }
        FFISafeExpr::FunctionCall(name, arg_start, len) => {
            box_drop(name);
            drop_all(arg_start, len);
        }
        FFISafeExpr::VariableRef(name) => {
            box_drop(name);
        }
        _ => {}
    }
    box_drop(expr.get_filename());
}

#[no_mangle]
pub unsafe extern "C" fn drop_all(start: *mut LocatedFFISafeExpr, len: usize) {
    let arr = Box::from_raw(std::slice::from_raw_parts_mut(start, len));
    for expr in arr.iter() {
        drop_expr(expr);
    }
}
