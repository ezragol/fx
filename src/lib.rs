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

// #[no_mangle]
// pub extern fn interface() -> lexer::Token {
//     lexer::Token::Number(Some(0), None)
// }