use std::{
    fs::File,
    io::{self, BufReader, Read},
};

use crate::errors::*;

#[repr(C)]
#[derive(Clone, Debug, PartialEq, Eq)]
pub enum Symbol {
    Multiply,
    Divide,
    Add,
    Subtract,
    ToPower,
    Comma,
    Equals,
    Colon,
    GreaterThan,
    LessThan,
    Ampersand,
    Pipe,
    Negate,
    Dot,
    Compound(Box<Symbol>, Box<Symbol>),
}

#[repr(C)]
#[derive(Clone, Debug, PartialEq, Eq)]
pub enum Bracket {
    Parens(Is),
    Square(Is),
    Curly(Is),
    DoubleQuote,
    SingleQuote,
    BackQuote,
}

// saving time sue me
#[repr(C)]
#[derive(Clone, Debug, PartialEq, Eq)]
pub enum Is {
    Open,
    Closed,
}

#[repr(C)]
#[derive(Clone, Debug, PartialEq)]
pub enum Token {
    Extern,
    Identifier(String),
    Number(Option<isize>, Option<f64>),
    When,
    Operation(Symbol),
    Let,
    Wall(Bracket),
    Newline,
    Grouping(Vec<Token>),
}

pub struct Interpreter {
    bytes: Vec<u8>,
    index: usize,
    size: usize,
}

// DEAL with newlines / how to figure out what is an expression

impl Interpreter {
    pub fn new(mut buf: BufReader<File>) -> io::Result<Interpreter> {
        let mut bytes = vec![];
        buf.read_to_end(&mut bytes)?;
        bytes.push(0);

        let size = bytes.len().clone();
        Ok(Interpreter {
            bytes,
            index: 0,
            size,
        })
    }

    fn next(&mut self) -> Result<char> {
        if self.index < self.size {
            let byte = self.bytes[self.index];
            self.index += 1;
            Ok(byte as char)
        } else {
            Err(EofError.into())
        }
    }

    fn parse_grouping(&mut self, bracket: Bracket) -> Result<Token> {
        match bracket {
            Bracket::Parens(Is::Closed) | Bracket::Square(Is::Closed) => {
                return Ok(Token::Wall(bracket));
            }
            _ => {}
        }

        let mut group = vec![Token::Wall(bracket)];
        loop {
            let next = self.parse_next()?;
            match next {
                Token::Wall(Bracket::Parens(Is::Closed))
                | Token::Wall(Bracket::Square(Is::Closed)) => {
                    group.push(next);
                    break;
                }
                Token::Newline => {}
                _ => {
                    group.push(next);
                }
            }
        }

        let open = group.first().unwrap();
        let close = group.last().unwrap();

        if let (Token::Wall(o), Token::Wall(c)) = (open, close) {
            if std::mem::discriminant(o) == std::mem::discriminant(c) {
                return Ok(Token::Grouping(group));
            }
        }
        return Err(GroupingError.into());
    }

    fn parse_symbol(next: char) -> Option<Symbol> {
        match next {
            '*' => Some(Symbol::Multiply),
            '/' => Some(Symbol::Divide),
            '+' => Some(Symbol::Add),
            '-' => Some(Symbol::Subtract),
            '^' => Some(Symbol::ToPower),
            ',' => Some(Symbol::Comma),
            '=' => Some(Symbol::Equals),
            ':' => Some(Symbol::Colon),
            '>' => Some(Symbol::GreaterThan),
            '<' => Some(Symbol::LessThan),
            '&' => Some(Symbol::Ampersand),
            '|' => Some(Symbol::Pipe),
            '!' => Some(Symbol::Negate),
            '.' => Some(Symbol::Dot),
            _ => None,
        }
    }

    fn ident(next: char) -> bool {
        let symbols = ['_', '$'];
        next.is_alphabetic() || symbols.contains(&next)
    }

    pub fn parse_next(&mut self) -> Result<Token> {
        let mut next = self.next()?;
        let mut identifier = String::new();

        while next.is_whitespace() {
            if next == '\n' {
                'b: loop {
                    next = self.next()?;
                    if next != '\n' {
                        self.index -= 1;
                        break 'b;
                    }
                }
                return Ok(Token::Newline);
            }
            next = self.next()?;
        }

        if Interpreter::ident(next) {
            loop {
                identifier.push(next);
                next = self.next()?;
                if !Interpreter::ident(next) && !next.is_numeric() {
                    self.index -= 1;
                    break;
                }
            }

            let token = match identifier.as_str() {
                "extern" => Token::Extern,
                "when" => Token::When,
                "let" => Token::Let,
                i => Token::Identifier(i.to_string()),
            };

            return Ok(token);
        }

        let mut num_str = None;
        while next.is_numeric() || next == '.' {
            num_str = match num_str {
                None => Some(String::new()),
                _ => num_str,
            };
            num_str.as_mut().unwrap().push(next);
            next = self.next()?;
        }

        if let Some(num) = num_str {
            let mut int_val: Option<isize> = None;
            let mut float_val = None;
            self.index -= 1;
            if num == "." {
                next = '.';
            } else {
                if num.contains('.') {
                    float_val = Some(num.parse::<f64>().unwrap());
                } else {
                    int_val = Some(num.parse::<isize>().unwrap());
                }
                return Ok(Token::Number(int_val, float_val));
            }
        }

        if next == '#' {
            loop {
                next = self.next()?;
                if next == '\n' || next == '\r' {
                    return self.parse_next();
                }
            }
        }

        // check binary operators
        let symbol = Interpreter::parse_symbol(next);

        if let Some(op) = symbol {
            if let Ok(another) = self.next() {
                if let Some(second) = Interpreter::parse_symbol(another) {
                    return Ok(Token::Operation(Symbol::Compound(op.into(), second.into())));
                }
                self.index -= 1;
            }
            return Ok(Token::Operation(op));
        }

        let bracket = match next {
            '(' => Some(Bracket::Parens(Is::Open)),
            '[' => Some(Bracket::Square(Is::Open)),
            ')' => Some(Bracket::Parens(Is::Closed)),
            ']' => Some(Bracket::Square(Is::Closed)),
            _ => None,
        };

        if let Some(br) = bracket {
            self.parse_grouping(br)
        } else {
            Err(UnknownTokenError.into())
        }
    }

    pub fn pull(&mut self) -> Result<Vec<Token>> {
        let mut tokens = vec![];
        loop {
            match self.parse_next() {
                Ok(token) => tokens.push(token),
                Err(e) => {
                    if !self.done() {
                        return Err(e);
                    } else {
                        return Ok(tokens);
                    }
                }
            }
        }
    }

    pub fn done(&self) -> bool {
        self.index == self.size
    }
}
