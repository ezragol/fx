use std::{
    fs::File,
    io::{self, BufReader, Read},
};

#[derive(Debug)]
pub enum BinaryOp {
    Multiply,
    Divide,
    Add,
    Subtract,
    ToPower,
    Separate,
    GreaterThan,
    LessThan,
    Assign,
    Declaration
}

#[derive(Debug)]
pub enum Bracket {
    Parens(Is),
    Square(Is),
    Curly(Is),
}

// saving time sue me
#[derive(Debug)]
pub enum Is {
    Open,
    Closed,
}

#[derive(Debug)]
pub enum Token {
    Extern,
    Identifier(String),
    Number(Option<isize>, Option<f64>),
    When,
    Default,
    Binary(BinaryOp),
    Let,
    Bracket(Bracket),
}

pub struct Interpreter {
    bytes: Vec<u8>,
    index: usize,
    size: usize,
}

impl Interpreter {
    pub fn new(mut buf: BufReader<File>) -> io::Result<Interpreter> {
        let mut bytes = vec![];
        buf.read_to_end(&mut bytes)?;

        let size = bytes.len().clone();
        Ok(Interpreter {
            bytes,
            index: 0,
            size,
        })
    }

    fn next(&mut self) -> Option<char> {
        if self.index < self.size {
            let byte = self.bytes[self.index];
            self.index += 1;
            return Some(byte as char);
        }
        return None;
    }

    pub fn parse_next(&mut self) -> Option<Token> {
        let mut next = ' ';
        let mut identifier = String::new();

        while next.is_whitespace() {
            next = self.next()?;
        }

        if next.is_alphabetic() {
            loop {
                identifier.push(next);
                next = self.next()?;
                if !next.is_alphanumeric() {
                    self.index -= 1;
                    break;
                }
            }

            let token = match identifier.as_str() {
                "extern" => Token::Extern,
                "when" => Token::When,
                "default" => Token::Default,
                "let" => Token::Let,
                i => Token::Identifier(i.to_string()),
            };

            return Some(token);
        }

        let mut num_str = None;
        while next.is_numeric() || next == '.' {
            num_str = match num_str {
                None => Some(String::new()),
                _ => num_str,
            };
            num_str.as_mut()?.push(next);
            next = self.next()?;
        }

        if let Some(num) = num_str {
            let mut int_val: Option<isize> = None;
            let mut float_val = None;
            self.index -= 1;

            if num.contains('.') {
                float_val = Some(num.parse::<f64>().unwrap());
            } else {
                int_val = Some(num.parse::<isize>().unwrap());
            }
            return Some(Token::Number(int_val, float_val));
        }

        if next == '#' {
            loop {
                next = self.next()?;
                if next == '\n' || next == '\r' {
                    return self.parse_next();
                }
            }
        }

        let binary_op = match next {
            '*' => Some(BinaryOp::Multiply),
            '/' => Some(BinaryOp::Divide),
            '+' => Some(BinaryOp::Add),
            '-' => Some(BinaryOp::Subtract),
            '^' => Some(BinaryOp::ToPower),
            ',' => Some(BinaryOp::Separate),
            '>' => Some(BinaryOp::GreaterThan),
            '<' => Some(BinaryOp::LessThan),
            '=' => Some(BinaryOp::Assign),
            ':' => Some(BinaryOp::Declaration),
            _ => None,
        };

        if let Some(binary) = binary_op {
            return Some(Token::Binary(binary));
        }

        let brackets = match next {
            '(' => Some(Bracket::Parens(Is::Open)),
            ')' => Some(Bracket::Parens(Is::Closed)),
            '[' => Some(Bracket::Square(Is::Open)),
            ']' => Some(Bracket::Square(Is::Closed)),
            '{' => Some(Bracket::Curly(Is::Open)),
            '}' => Some(Bracket::Curly(Is::Closed)),
            _ => None,
        };
        return Some(Token::Bracket(brackets?));
    }
}
