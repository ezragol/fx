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

    fn basic_op_prec(symbol: Symbol) -> Option<u8> {
        match symbol {
            Symbol::ToPower => Some(0),
            Symbol::Multiply => Some(1),
            Symbol::Divide => Some(2),
            Symbol::Add => Some(3),
            Symbol::Subtract => Some(4),
            Symbol::LessThan => Some(5),
            Symbol::GreaterThan => Some(6),
            Symbol::Comma => Some(7),
            Symbol::Equals => Some(8),
            _ => None,
        }
    }

    fn compound_op_prec(symbols: (Symbol, Symbol)) -> Option<usize> {
        match symbols {
            (Symbol::LessThan, Symbol::Equals) => Some(0),
            (Symbol::GreaterThan, Symbol::Equals) => Some(1),
            (Symbol::Equals, Symbol::Equals) => Some(2),
            (Symbol::Negate, Symbol::Equals) => Some(3),
            (Symbol::Ampersand, Symbol::Ampersand) => Some(4),
            (Symbol::Pipe, Symbol::Pipe) => Some(5),
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

        if let Token::Symbol(Symbol::Comma) = self.next_token()? {
            if let Ok(end) = self.expect_num() {
                return Ok(Range::new(Box::new(min), Box::new(end)));
            } else {
                self.back();
                if let Token::Bracket(Bracket::Square(Is::Closed)) = self.next_token()? {
                    return Ok(Range::new(Box::new(min), Box::new(max)));
                }
            }
        }
        Err(RangeError.into())
    }

    fn parse_expr_or_err(&mut self) -> Result<Expr> {
        if let Some(expr) = self.parse_expression() {
            Ok(expr)
        } else {
            Err(DeclarationError.into())
        }
    }

    fn parse_definition(&mut self) -> Result<Expr> {
        let identifier = self.expect_identifier()?;
        let next = self.next_token()?;
        match next {
            Token::Grouping(args) => Ok(Box::new(FunctionDefinition::new(
                identifier,
                Parser::parse_def_args(args)?,
                self.parse_expr_or_err()?,
            ))),
            Token::Symbol(Symbol::Equals) => {
                self.back();
                Ok(Box::new(VariableDefinition::new(
                    identifier,
                    self.parse_expr_or_err()?,
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
                Token::Symbol(_) => Some(behind),
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

    fn read_expr_tokens(&mut self) -> Vec<Token> {
        let mut tokens = vec![];
        loop {
            if let Ok(token) = self.next_token() {
                match token {
                    Token::Newline => {
                        let next = self.next_token();
                        if let Ok(Token::Let) = next {
                            self.back();
                            break;
                        } else if let Ok(Token::Symbol(_)) = next {
                            if self.behind_is_expr().is_none() {
                                break;
                            }
                        } else if self.behind_is_operation().is_none() {
                            break;
                        }
                    }
                    Token::Symbol(_)
                    | Token::Identifier(_)
                    | Token::Number(_, _)
                    | Token::Grouping(_)
                    | Token::When
                    | Token::FunctionCall(_, _) => {
                        if tokens.len() > 0 || !matches!(token, Token::Symbol(Symbol::Equals)) {
                            tokens.push(token);
                        }
                    }
                    _ => {
                        break;
                    }
                }
            } else {
                break;
            }
        }
        return tokens;
    }

    fn parse_grouping(tokens: Vec<Token>, with_brackets: bool) -> Option<Expr> {
        let change = if with_brackets { 1 } else { 0 };

        if tokens.len() > 2 * change {
            let mut parser = Parser::from_tree(tokens[change..tokens.len() - change].to_vec());
            Some(parser.parse_expression()?)
        } else {
            None
        }
    }

    fn parse_non_symbol(&mut self, symbol: Token) -> Option<Expr> {
        match symbol {
            Token::Identifier(ident) => Some(Box::new(VariableRef::new(ident))),
            Token::Number(int, float) => Some(Box::new(NumberLiteral::new(
                int.is_none(),
                int.unwrap_or(0),
                float.unwrap_or(0.0),
            ))),
            Token::Grouping(tokens) => Parser::parse_grouping(tokens, true),
            // fix
            Token::FunctionCall(name, tokens) => Some(Box::new(
                if let Ok(args) = Parser::parse_call_args(tokens) {
                    FunctionCall::new(name, args)
                } else {
                    FunctionCall::new(name, vec![])
                },
            )),
            _ => None,
        }
    }

    fn parse_expression(&mut self) -> Option<Expr> {
        let tokens = self.read_expr_tokens();
        if tokens.len() == 0 {
            return None;
        }

        let mut reader = Parser::from_tree(tokens.clone());
        let mut op_indexes = vec![];
        let mut tree = None;
        let mut last_index = 0;

        loop {
            let next = reader.next_token();
            match next {
                // auto unwrap because otherwise it would not have even been added to token list
                Ok(Token::Symbol(op)) => {
                    op_indexes.push((reader.index - 1, Parser::basic_op_prec(op).unwrap()))
                }
                Ok(_) => {}
                Err(_) => {
                    break;
                }
            }
        }

        op_indexes.sort_by(|a, b| a.1.cmp(&b.1));

        for (index, prec) in op_indexes {
            if tree.is_none() {
                tree = Some(BinaryOperation::new(
                    prec,
                    self.parse_non_symbol(tokens[index - 1].clone())?,
                    self.parse_non_symbol(tokens[index + 1].clone())?,
                ));
                last_index = index;
            } else {
                if last_index < index {
                    tree = Some(BinaryOperation::new(
                        prec,
                        Box::new(tree.unwrap()),
                        self.parse_non_symbol(tokens[index + 1].clone())?,
                    ));
                } else {
                    tree = Some(BinaryOperation::new(
                        prec,
                        self.parse_non_symbol(tokens[index - 1].clone())?,
                        Box::new(tree.unwrap()),
                    ));
                }
                last_index = index;
            }
        }

        if tree.is_none() {
            return Some(self.parse_non_symbol(tokens[0].clone())?);
        }

        Some(Box::new(tree.unwrap()))
    }

    fn parse_def_args(args: Vec<Token>) -> Result<Vec<String>> {
        let mut arg_tree = vec![];
        let mut p = Parser::from_tree(args);

        p.forward();
        while let Ok(identifier) = p.expect_identifier() {
            if let Ok(next) = p.next_token() {
                match next {
                    Token::Symbol(Symbol::Comma) => {
                        arg_tree.push(identifier);
                    }
                    Token::Bracket(Bracket::Parens(Is::Closed)) => {
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
        return Err(IdentifierError.into());
    }

    fn parse_call_args(args: Vec<Token>) -> Result<Vec<Expr>> {
        let mut arg_tree = vec![];
        let mut p = Parser::from_tree(args);
        let mut current_group = vec![];
        p.forward();

        while let Ok(next) = p.next_token() {
            match next {
                Token::Symbol(Symbol::Comma) | Token::Bracket(Bracket::Parens(Is::Closed)) => {
                    if let Some(parsed) = Parser::parse_grouping(current_group.clone(), false) {
                        arg_tree.push(parsed);
                        current_group = vec![];
                    } else {
                        break;
                    }

                    // really fucking interesting
                    if let Token::Bracket(Bracket::Parens(Is::Closed)) = next {
                        return Ok(arg_tree);
                    }
                }
                _ => {
                    current_group.push(next);
                }
            }
        }
        return Err(BadCommaError.into());
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
                            match self.parse_expr_or_err() {
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
