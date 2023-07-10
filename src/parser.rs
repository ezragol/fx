use std::{
    fs::File,
    io::{self, BufReader},
};

use crate::{ast::*, errors::*, lexer::*};

pub struct Parser {
    lexer: Interpreter,
    ast: Vec<Expr>,
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
            Symbol::Comma => Some(7),
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

    fn next_token(&mut self) -> Result<Token> {
        let next = self.lexer.parse_next();
        if let Some(token) = next {
            return Ok(token);
        }
        Err(EofError.into())
    }

    fn expect_num(&mut self) -> Result<NumberLiteral> {
        let token = self.next_token()?;
        if let Token::Number(int, float) = token {
            return Ok(NumberLiteral::new(
                float.is_some(),
                int.unwrap_or(0),
                float.unwrap_or(0.0),
            ));
        }
        Err(RangeError.into())
    }

    fn expect_identifier(&mut self) -> Result<String> {
        let token = self.next_token()?;
        if let Token::Identifier(identifier) = token {
            return Ok(identifier);
        }
        Err(IdentifierError.into())
    }

    fn parse_range(&mut self) -> Result<Range> {
        let mut min = NumberLiteral::new(false, std::isize::MIN, 0.0);
        let max = NumberLiteral::new(false, std::isize::MAX, 0.0);

        if let Ok(start) = self.expect_num() {
            min = start;
        } else {
            self.lexer.push(-1);
        }

        if let Token::Operation(Symbol::Comma) = self.next_token()? {
            if let Ok(end) = self.expect_num() {
                return Ok(Range::new(Box::new(min), Box::new(end)));
            } else {
                self.lexer.push(-1);
                if let Token::Wall(Bracket::Square(Is::Closed)) = self.next_token()? {
                    return Ok(Range::new(Box::new(min), Box::new(max)));
                }
            }
        }
        Err(RangeError.into())
    }

    fn parse_definition(&mut self) -> Result<Expr> {
        let identifier = self.expect_identifier()?;
        let next = self.next_token()?;
        match next {
            Token::Wall(Bracket::Parens(Is::Open)) => Ok(Box::new(FunctionDefinition::new(
                identifier,
                self.parse_args()?,
                self.parse_block(true)?,
            ))),
            Token::Operation(Symbol::Equals) => Ok(Box::new(VariableDefinition::new(
                identifier,
                self.parse_expression()?,
            ))),
            _ => Err(UnexpectedTokenError::new(Token::Operation(Symbol::Equals)).into()),
        }
    }

    fn read_tokens(&mut self) -> Result<Vec<Token>> {
        let mut tokens = vec![];
        loop {
            let maybe_next = self.next_token();
            if let Ok(next) = maybe_next {
                if let Token::Newline = next {
                    self.lexer.push(-3);
                    let look_behind = self.next_token()?;
                    match look_behind {
                        Token::Operation(_)
                        | Token::Wall(Bracket::Parens(Is::Open))
                        | Token::Wall(Bracket::Square(Is::Open)) => {
                            self.lexer.push(2);
                        }
                        _ => {
                            self.lexer.push(1);
                            let look_ahead = self.next_token()?;
                            match look_ahead {
                                Token::Operation(_)
                                | Token::Wall(Bracket::Parens(Is::Closed))
                                | Token::Wall(Bracket::Square(Is::Closed)) => {
                                    self.lexer.push(-1);
                                }
                                _ => {
                                    tokens.push(next);
                                }
                            }
                        }
                    }
                } else {
                    tokens.push(next);
                }
            } else {
                return Ok(tokens);
            }
        }
    }

    fn parse_expression(&mut self) -> Result<Expr> {
        let mut tokens = self.read_tokens()?;
        println!("{:?}", tokens);
        Ok(Box::new(NumberLiteral::new(false, 0, 0.0)))
    }

    fn parse_block(&mut self, with_assignment: bool) -> Result<Vec<Expr>> {
        if let Token::Operation(Symbol::Equals) = self.next_token()? {
            return Ok(vec![]);
        }
        Err(UnexpectedTokenError::new(Symbol::Equals).into())
    }

    // parse arguments and consume closing parentheses
    fn parse_args(&mut self) -> Result<Vec<(String, Option<Expr>)>> {
        let mut args = vec![];
        while let Ok(identifier) = self.expect_identifier() {
            match self.next_token()? {
                Token::Wall(Bracket::Parens(Is::Closed)) => {
                    args.push((identifier, None));
                    return Ok(args);
                }
                Token::Operation(Symbol::Comma) => {
                    args.push((identifier, None));
                }
                Token::Operation(Symbol::Colon) => {
                    match self.next_token()? {
                        Token::Wall(Bracket::Square(Is::Open)) => {
                            args.push((identifier, Some(Box::new(self.parse_range()?))));
                            if let Token::Wall(Bracket::Parens(Is::Closed)) = self.next_token()? {
                                return Ok(args);
                            } else {
                                self.lexer.push(1);
                            }
                        }
                        Token::Identifier(id) => {
                            // todo
                            self.lexer.parse_next();
                        }
                        _ => {
                            return Err(IdentifierError.into());
                        }
                    }
                }
                _ => {
                    return Err(UnexpectedTokenError::new(Symbol::Comma).into());
                }
            }
        }
        Err(EofError.into())
    }

    pub fn run(&mut self) {
        let mut next: Option<Token> = None;
        let mut first = true;
        let mut tree: Vec<Expr> = vec![];

        loop {
            if next.is_none() && !first {
                if !self.lexer.done() {
                    panic!("Unknown symbol!");
                }
                break;
            } else if !first {
                if let Token::Let = next.clone().unwrap() {
                    match self.parse_definition() {
                        Err(e) => {
                            panic!("{}", e);
                        }
                        Ok(d) => {
                            tree.push(d);
                        }
                    }
                }
            }

            first = false;
            next = self.lexer.parse_next();
        }
        println!("{:#?}", tree);
    }
}
