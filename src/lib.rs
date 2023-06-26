mod ast;
mod lexer;
mod parser;
mod errors;

#[cfg(test)]
mod test;

#[no_mangle]
pub extern fn interface() -> lexer::Token {
    lexer::Token::Number(Some(0), None)
}