use std::{
    fs::File,
    io::{self, BufReader, Read},
};

#[repr(C)]
#[derive(Clone, Debug)]
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
}

#[repr(C)]
#[derive(Clone, Debug)]
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
#[derive(Clone, Debug)]
pub enum Is {
    Open,
    Closed,
}

#[repr(C)]
#[derive(Clone, Debug)]
pub enum Token {
    Extern,
    Identifier(String),
    Number(Option<isize>, Option<f64>),
    When,
    Operation(Symbol),
    Let,
    Wall(Bracket),
    Newline,
    Grouping(Vec<Token>)
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

    fn next(&mut self) -> Option<char> {
        if self.index < self.size {
            let byte = self.bytes[self.index];
            self.index += 1;
            return Some(byte as char);
        }
        return None;
    }

    fn parse_grouping(&mut self, bracket: Bracket) -> Option<Token> {
        match bracket {
            Bracket::Parens(Is::Closed) | Bracket::Square(Is::Closed) => {
                return Some(Token::Wall(bracket));
            },
            _ => {}
        }
        let mut group = vec![Token::Wall(bracket)];
        loop {
            let next = self.parse_next();
            match next {
                Some(Token::Wall(Bracket::Parens(Is::Closed)))
                | Some(Token::Wall(Bracket::Square(Is::Closed))) => {
                    group.push(next?);
                    break;
                }
                Some(Token::Newline) => {}
                Some(_) => {
                    group.push(next?);
                },
                None => {
                    break;
                }
            }
        }
        return Some(Token::Grouping(group));
    }

    pub fn parse_next(&mut self) -> Option<Token> {
        let mut next = self.next()?;
        let mut identifier = String::new();

        while next.is_whitespace() {
            if next == '\n' {
                loop {
                    next = self.next()?;
                    if next != '\n' {
                        self.index -= 1;
                        break;
                    }
                }
                return Some(Token::Newline);
            }
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

        // check binary operators

        let symbol = match next {
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
            _ => None,
        };

        if let Some(op) = symbol {
            return Some(Token::Operation(op));
        }
        
        let bracket = match next {
            '(' => Some(Bracket::Parens(Is::Open)),
            '[' => Some(Bracket::Square(Is::Open)),
            ')' => Some(Bracket::Parens(Is::Closed)),
            ']' => Some(Bracket::Square(Is::Closed)),
            _ => None
        };

        // returns none if the current token hasn't matched anything yet
        return self.parse_grouping(bracket?);
    }

    pub fn pull(&mut self) -> Vec<Token> {
        let mut tokens = vec![];
        while let Some(token) = self.parse_next() {
            tokens.push(token);
        }
        println!("{:#?}", tokens);
        return tokens;
    }

    pub fn done(&self) -> bool {
        self.index == self.size
    }
}
