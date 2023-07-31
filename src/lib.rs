use ast::*;
use parser::Parser;

mod ast;
mod errors;
mod lexer;
mod parser;

#[macro_export]
macro_rules! err {
    ($($t:tt)*) => {{
        eprintln!($($t)*);
        std::process::exit(1);
    }};
}

#[cfg(test)]
mod test;

#[no_mangle]
pub extern "C" fn recieve_tokens() -> FFISafeExprVec {
    let parser = Parser::new("test.txt");
    let tree = match parser {
        Ok(mut p) => p.run(),
        Err(e) => err!("{}", e),
    };
    let ffi_safe_tree = convert_vec(tree);
    let ffi_safe = FFISafeExprVec {
        ptr: ffi_safe_tree.0,
        len: ffi_safe_tree.1,
    };
    return ffi_safe;
}

unsafe fn box_drop<T>(ffi_val: *const T) {
    drop(Box::from_raw(ffi_val.cast_mut()));
}

unsafe fn drop_expr(expr: *const FFISafeExpr) {
    match *expr {
        FFISafeExpr::StringLiteral(s) => {
            box_drop(s);
        }
        FFISafeExpr::FunctionDefinition(name, arg_start, len, body) => {
            let args = std::slice::from_raw_parts(arg_start, len);
            box_drop(name);
            for arg in args {
                box_drop(arg);
            }
            drop_expr(body);
        }
        FFISafeExpr::ChainExpression(chain_start, len) => {
            drop_all(chain_start, len);
        }
        FFISafeExpr::BinaryOperation(_, left, right) => {
            drop_expr(left);
            drop_expr(right);
        }
        FFISafeExpr::WhenExpression(predicate, result) => {
            drop_expr(predicate);
            drop_expr(result);
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
    box_drop(expr);
}

#[no_mangle]
pub unsafe extern "C" fn drop_all(start: *const FFISafeExpr, len: usize) {
    let arr = std::slice::from_raw_parts(start, len);
    for expr in arr {
        drop_expr(expr);
    }
}
