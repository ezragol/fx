use std::{
    fs::File,
    io::{self, BufReader},
};

use crate::{
    ast::Expression,
    lexer::{Interpreter, Symbol, Token},
};

pub struct Parser {
    lexer: Interpreter,
    ast: Vec<Box<dyn Expression>>,
}

impl Parser {
    pub fn new(src: &str) -> io::Result<Parser> {
        let file = BufReader::new(File::open(src)?);
        let lexer = Interpreter::new(file)?;
        Ok(Parser { lexer, ast: vec![] })
    }

    fn basic_op_prec(symbol: Symbol) -> Option<usize> {
        match symbol {
            Symbol::ToPower => Some(0),
            Symbol::Multiply => Some(1),
            Symbol::Divide => Some(1),
            Symbol::Add => Some(2),
            Symbol::Subtract => Some(2),
            Symbol::LessThan => Some(3),
            Symbol::GreaterThan => Some(3),
            Symbol::Separate => Some(7),
            _ => None,
        }
    }

    fn compound_op_prec(symbols: (Symbol, Symbol)) -> Option<usize> {
        match symbols {
            (Symbol::LessThan, Symbol::Equals) => Some(3),
            (Symbol::GreaterThan, Symbol::Equals) => Some(3),
            (Symbol::Equals, Symbol::Equals) => Some(4),
            (Symbol::Negate, Symbol::Equals) => Some(4),
            (Symbol::Ampersand, Symbol::Ampersand) => Some(5),
            (Symbol::Pipe, Symbol::Pipe) => Some(6),
            _ => None,
        }
    }

    pub fn run(&mut self) {
        let mut next: Option<Token> = None;
        let mut first = true;
        loop {
            if next.is_none() && !first {
                if !self.lexer.done() {
                    panic!("Unknown symbol!");
                }
                break;
            } else if !first {
                println!("{:?}", next.as_ref().unwrap());
            }

            first = false;
            next = self.lexer.parse_next();
        }
    }
}
