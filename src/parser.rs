use std::{
    fs::File,
    io::{self, BufReader},
};

use crate::err;

use crate::{ast::*, errors::*, lexer::*};

pub struct Parser {
    ast: Vec<Expr>,
    tokens: Vec<Token>,
    index: usize,
}

impl Parser {
    pub fn new(src: &str) -> Result<Parser> {
        let file = BufReader::new(File::open(src)?);
        let mut lexer = Interpreter::new(file)?;

        Ok(Parser {
            ast: vec![],
            tokens: lexer.pull()?,
            index: 0,
        })
    }

    pub fn from_tree(tokens: Vec<Token>) -> Parser {
        Parser {
            ast: vec![],
            tokens,
            index: 0,
        }
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
        if self.index >= self.tokens.len() {
            Err(EofError.into())
        } else {
            let next = self.tokens[self.index].clone();
            self.forward();
            Ok(next)
        }
    }

    fn expect_num(&mut self) -> Result<NumberLiteral> {
        let token = self.look_ahead()?;
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
        let token = self.look_ahead()?;
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
            self.back();
        }

        if let Token::Operation(Symbol::Comma) = self.next_token()? {
            if let Ok(end) = self.expect_num() {
                return Ok(Range::new(Box::new(min), Box::new(end)));
            } else {
                self.back();
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
            Token::Grouping(args) => Ok(Box::new(FunctionDefinition::new(
                identifier,
                self.parse_args(args)?,
                self.parse_expression()?,
            ))),
            Token::Operation(Symbol::Equals) => {
                self.back();
                Ok(Box::new(VariableDefinition::new(
                    identifier,
                    self.parse_expression()?,
                )))
            }
            _ => Err(DeclarationError.into()),
        }
    }

    fn look_ahead(&mut self) -> Result<Token> {
        loop {
            let token = self.next_token()?;
            match token {
                Token::Newline => {}
                _ => {
                    return Ok(token);
                }
            }
        }
    }

    fn look_behind(&mut self) -> Result<Token> {
        loop {
            self.push(-2);
            let token = self.next_token()?;
            match token {
                Token::Newline => {}
                _ => {
                    self.forward();
                    return Ok(token);
                }
            }
        }
    }

    fn behind_is_expr(&mut self) -> Option<Token> {
        if let Ok(behind) = self.look_behind() {
            match behind {
                Token::Identifier(_) | Token::Grouping(_) => Some(behind),
                _ => None,
            }
        } else {
            None
        }
    }

    fn behind_is_operation(&mut self) -> Option<Token> {
        if let Ok(behind) = self.look_behind() {
            match behind {
                Token::Operation(_) => Some(behind),
                _ => None,
            }
        } else {
            None
        }
    }

    fn push(&mut self, amount: isize) {
        if amount < 0 {
            let abs = amount.abs() as usize;
            if abs > self.index {
                self.index = 0
            } else {
                self.index -= abs;
            }
        } else {
            self.index += amount as usize;
        }
    }

    fn back(&mut self) {
        self.push(-1);
    }

    fn forward(&mut self) {
        self.push(1);
    }

    fn read_expr_tokens(&mut self) -> Result<Vec<Token>> {
        let mut tokens = vec![];
        loop {
            if let Ok(token) = self.next_token() {
                match token {
                    Token::Newline => {
                        let next = self.next_token();
                        if let Ok(Token::Let) = next {
                            self.back();
                            break;
                        } else if let Ok(Token::Operation(_)) = next {
                            if let None = self.behind_is_expr() {
                                break;
                            }
                        } else if let None = self.behind_is_operation() {
                            break;
                        }
                    }
                    Token::Operation(_)
                    | Token::Identifier(_)
                    | Token::Number(_, _)
                    | Token::Grouping(_)
                    | Token::When => {
                        tokens.push(token);
                    }
                    _ => {
                        break;
                    }
                }
            } else {
                break;
            }
        }
        return Ok(tokens);
    }

    fn parse_expression(&mut self) -> Result<Expr> {
        let mut tokens = self.read_expr_tokens()?;
        println!("{:#?}", tokens);
        tokens.pop();
        for token in tokens {}
        Ok(Box::new(NumberLiteral::new(false, 0, 0.0)))
    }

    fn parse_args(&mut self, args: Vec<Token>) -> Result<Vec<String>> {
        let mut arg_tree = vec![];
        let mut a = Parser::from_tree(args);
        a.forward();

        while let Ok(identifier) = a.expect_identifier() {
            if let Ok(next) = a.next_token() {
                match next {
                    Token::Operation(Symbol::Comma) => {
                        arg_tree.push(identifier);
                    }
                    Token::Wall(Bracket::Parens(Is::Closed)) => {
                        arg_tree.push(identifier);
                        return Ok(arg_tree);
                    }
                    // type annotations go here
                    // Token::Operation(Symbol::Colon) => {}
                    // n: [0,]
                    _ => {
                        break;
                    }
                }
            }
        }
        Err(IdentifierError.into())
    }

    pub fn run(&mut self) {
        let mut next: Option<Token> = None;
        let mut first = true;
        let mut tree: Vec<Expr> = vec![];

        loop {
            if next.is_none() && !first {
                break;
            } else if !first {
                if let Token::Let = next.clone().unwrap() {
                    match self.parse_definition() {
                        Err(e) => {
                            err!("error: {}", e);
                        }
                        Ok(d) => {
                            tree.push(d);
                        }
                    }
                } else {
                    match next.unwrap() {
                        Token::Identifier(_) | Token::Extern | Token::Grouping(_) => {
                            self.back();
                            match self.parse_expression() {
                                Err(e) => {
                                    err!("error: {}", e);
                                }
                                Ok(d) => {
                                    tree.push(d);
                                }
                            }
                        }
                        _ => {}
                    }
                }
            }

            first = false;
            if let Ok(token) = self.next_token() {
                next = Some(token);
            } else {
                next = None;
            }
        }

        println!("{:#?}", tree);
    }
}
