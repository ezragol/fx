use std::{io::BufReader, fs::File, time};

use lexer::Interpreter;

// mod ast;
mod lexer;

fn main() {
    let file = BufReader::new(File::open("./test.txt").unwrap());
    let mut lexer = Interpreter::new(file).unwrap();
    let now = time::Instant::now();
    loop {
        let next = lexer.parse_next();
        if next.is_none() {
            break;
        }
        println!("{:?}", next.unwrap());
    }
    println!("{:?}", now.elapsed());
}
