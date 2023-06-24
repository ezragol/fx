use std::time;

use crate::parser::Parser;

mod ast;
mod lexer;
mod parser;

fn main() {
    let now = time::Instant::now();
    let mut parser = Parser::new("test.txt").unwrap();
    parser.run();

    println!("{:?}", now.elapsed());
}
