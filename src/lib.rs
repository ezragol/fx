use ast::*;
use parser::Parser;

mod ast;
mod lexer;
mod parser;
mod errors;

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
pub extern fn recieve_tokens() -> FFISafeExprVec {
    let parser = Parser::new("test.txt");
    let tree = match parser {
        Ok(mut p) => p.run(),
        Err(e) => err!("{}", e)
    };
    // println!("{:#?}", tree);
    let ffi_safe_tree = convert_vec(tree);
    let ffi_safe = FFISafeExprVec {
        ptr: ffi_safe_tree.0,
        len: ffi_safe_tree.1,
    };
    return ffi_safe;
}