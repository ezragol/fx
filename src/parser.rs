use std::{fs::File, io::BufReader};

use crate::err;

use crate::{ast::*, errors::*, lexer::*};

pub struct Parser {
    tokens: Vec<Token>,
    index: usize,
}

#[cfg(test)]
mod tests {
    use super::*;
    const BASIC_OP_COUNT: u8 = 9;
    const TEST_INPUT: &str = "src/tests/";

    fn path(add: &str) -> String {
        format!("{}/{}.txt", TEST_INPUT, add)
    }

    fn parser_from(file: &str) -> Result<Parser> {
        Parser::new(&path(file))
    }

    #[test]
    fn new() {
        let parser = parser_from("basic");
        assert!(parser.is_ok());
    }

    #[test]
    fn basic_op_prec() {
        let has_op_prec = [
            Symbol::ToPower,
            Symbol::Multiply,
            Symbol::Divide,
            Symbol::Add,
            Symbol::Subtract,
            Symbol::LessThan,
            Symbol::GreaterThan,
            Symbol::Comma,
            Symbol::Equals,
        ];
        for (i, symbol) in has_op_prec.into_iter().enumerate() {
            assert_eq!(Parser::basic_op_prec(symbol).unwrap(), i as u8);
        }

        let no_op_prec = [
            Symbol::Ampersand,
            Symbol::Colon,
            Symbol::Pipe,
            Symbol::Negate,
            Symbol::Dot,
        ];
        for symbol in no_op_prec {
            assert!(Parser::basic_op_prec(symbol).is_none());
        }
    }

    #[test]
    fn compound_op_prec() {
        let compounds = [
            (Symbol::LessThan, Symbol::Equals),
            (Symbol::GreaterThan, Symbol::Equals),
            (Symbol::Equals, Symbol::Equals),
            (Symbol::Negate, Symbol::Equals),
            (Symbol::Ampersand, Symbol::Ampersand),
            (Symbol::Pipe, Symbol::Pipe),
        ];
        for (i, symbol) in compounds.into_iter().enumerate() {
            assert_eq!(
                Parser::compound_op_prec(symbol).unwrap(),
                i as u8 + BASIC_OP_COUNT
            );
        }
    }

    #[test]
    fn next_token() {
        // stupid
        let mut parser = parser_from("basic").unwrap();
        let expected = [
            Token::Let,
            Token::Identifier("age".to_string()),
            Token::Symbol(Symbol::Equals),
            Token::Number(Some(0), Some(17.0)),
        ];
        for token in expected {
            assert_eq!(parser.next_token().unwrap(), token);
        }
        assert!(parser.next_token().is_err());
    }

    #[test]
    fn expect_identifier() {
        let mut parser = parser_from("basic").unwrap();
        assert_eq!(parser.expect_identifier().unwrap(), "a".to_string());
    }

    #[test]
    fn look_ahead() {
        let tokens = vec![Token::Let, Token::Newline, Token::Let];
        let mut parser = Parser::from_tree(tokens);
        assert_eq!(parser.look_ahead().unwrap(), Token::Let);
        assert_eq!(parser.look_ahead().unwrap(), Token::Let);
        assert!(parser.look_ahead().is_err());
    }
}

impl Parser {
    pub fn new(src: &str) -> Result<Parser> {
        let file = BufReader::new(File::open(src)?);
        let mut lexer = Interpreter::new(file)?;

        Ok(Parser {
            tokens: lexer.pull()?,
            index: 0,
        })
    }

    // no coverage
    pub fn from_tree(tokens: Vec<Token>) -> Parser {
        Parser { tokens, index: 0 }
    }

    // if you change this, make sure to change BASIC_OP_COUNT above
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

    fn compound_op_prec(symbols: (Symbol, Symbol)) -> Option<u8> {
        match symbols {
            (Symbol::LessThan, Symbol::Equals) => Some(9),
            (Symbol::GreaterThan, Symbol::Equals) => Some(10),
            (Symbol::Equals, Symbol::Equals) => Some(11),
            (Symbol::Negate, Symbol::Equals) => Some(12),
            (Symbol::Ampersand, Symbol::Ampersand) => Some(13),
            (Symbol::Pipe, Symbol::Pipe) => Some(14),
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

    fn expect_identifier(&mut self) -> Result<String> {
        let token = self.look_ahead()?;
        if let Token::Identifier(identifier) = token {
            return Ok(identifier);
        }
        Err(IdentifierError.into())
    }

    // no coverage
    fn parse_expr_or_err(&mut self) -> Result<Expr> {
        match self.parse_expression() {
            Some(expr) => Ok(expr),
            _ => Err(DeclarationError.into()),
        }
    }

    fn parse_definition(&mut self) -> Result<Expr> {
        let identifier = self.expect_identifier()?;
        let next = self.next_token()?;
        match next {
            Token::Grouping(args) => Ok(Expr::FunctionDefinition(
                identifier,
                Parser::parse_def_args(args)?,
                self.parse_expr_or_err()?.into(),
            )),
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
                    | Token::String(_)
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
            Token::Identifier(ident) => Some(Expr::VariableRef(ident)),
            Token::Number(int, float) => Some(Expr::NumberLiteral(
                int.is_none(),
                int.unwrap_or(0),
                float.unwrap_or(0.0),
            )),
            Token::String(s) => Some(Expr::StringLiteral(s)),
            Token::Grouping(tokens) => Parser::parse_grouping(tokens, true),
            // fix
            Token::FunctionCall(name, tokens) => {
                Some(if let Ok(args) = Parser::parse_chain(tokens, false) {
                    Expr::FunctionCall(name, args)
                } else {
                    Expr::FunctionCall(name, vec![])
                })
            }
            _ => None,
        }
    }

    fn branch(&mut self, tokens: Vec<Token>, mut indexes: Vec<(usize, u8)>) -> Option<Expr> {
        let mut last_index = 0;
        let mut tree = None;
        indexes.sort_by(|a, b| a.1.cmp(&b.1));

        for (index, prec) in indexes {
            if tree.is_none() {
                tree = Some(Expr::BinaryOperation(
                    prec,
                    self.parse_non_symbol(tokens[index - 1].clone())?.into(),
                    self.parse_non_symbol(tokens[index + 1].clone())?.into(),
                ));
                last_index = index;
            } else {
                if last_index < index {
                    tree = Some(Expr::BinaryOperation(
                        prec,
                        tree.unwrap().into(),
                        self.parse_non_symbol(tokens[index + 1].clone())?.into(),
                    ));
                } else {
                    tree = Some(Expr::BinaryOperation(
                        prec,
                        self.parse_non_symbol(tokens[index - 1].clone())?.into(),
                        tree.unwrap().into(),
                    ));
                }
                last_index = index;
            }
        }

        if tree.is_none() {
            return Some(self.parse_non_symbol(tokens[0].clone())?);
        }

        Some(tree.unwrap())
    }

    fn parse_expression(&mut self) -> Option<Expr> {
        let tokens = self.read_expr_tokens();
        if tokens.len() == 0 {
            return None;
        }

        if tokens.contains(&Token::Symbol(Symbol::Comma)) {
            let expressions = Parser::parse_chain(tokens, false).unwrap();
            return Some(Expr::ChainExpression(expressions));
        }

        let mut reader = Parser::from_tree(tokens.clone());
        let mut op_indexes = vec![];
        let mut when_index = 0;

        loop {
            let next = reader.next_token();
            match next {
                Ok(Token::When) => {
                    when_index = reader.index - 1;
                }
                Ok(Token::Symbol(Symbol::Compound(first, second))) => {
                    op_indexes.push((
                        reader.index - 1,
                        Parser::compound_op_prec((*first, *second)).unwrap(),
                    ));
                }
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

        if when_index > 0 {
            let left: Vec<(usize, u8)> = op_indexes
                .clone()
                .into_iter()
                .filter(|(index, _)| index < &when_index)
                .collect();
            let right: Vec<(usize, u8)> = op_indexes
                .into_iter()
                .filter(|(index, _)| index > &when_index)
                .map(|(index, prec)| (index - when_index - 1, prec))
                .collect();

            return Some(Expr::WhenExpression(
                self.branch(tokens[when_index + 1..].to_vec(), right)?
                    .into(),
                self.branch(tokens[..when_index].to_vec(), left)?.into(),
            ));
        } else {
            self.branch(tokens, op_indexes)
        }
    }

    fn parse_def_args(args: Vec<Token>) -> Result<Vec<String>> {
        let mut arg_tree = vec![];
        let mut p = Parser::from_tree(args.clone());

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

    fn parse_one_of_chain(current_group: Vec<Token>) -> Result<Expr> {
        if let Some(parsed) = Parser::parse_grouping(current_group.clone(), false) {
            Ok(parsed)
        } else {
            Err(BadCommaError.into())
        }
    }

    fn parse_chain(args: Vec<Token>, is_arg: bool) -> Result<Vec<Expr>> {
        let mut arg_tree = vec![];
        let mut p = Parser::from_tree(args);
        let mut current_group = vec![];
        if is_arg {
            p.forward();
        }

        while let Ok(next) = p.next_token() {
            match next {
                Token::Symbol(Symbol::Comma) | Token::Bracket(Bracket::Parens(Is::Closed)) => {
                    let parsed = Parser::parse_one_of_chain(current_group)?;
                    arg_tree.push(parsed);
                    current_group = vec![];

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

        if is_arg {
            Err(BadCommaError.into())
        } else {
            arg_tree.push(Parser::parse_one_of_chain(current_group)?);
            Ok(arg_tree)
        }
    }

    pub fn run(&mut self) -> Vec<Expr> {
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
                        Token::Identifier(_)
                        | Token::Extern
                        | Token::Grouping(_)
                        | Token::FunctionCall(_, _) => {
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

        return tree;
    }
}
