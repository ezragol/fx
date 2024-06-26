use crate::{ast::*, errors::*, lexer::*};
use std::{collections::HashMap, error::Error, fs::File, io::BufReader};

pub struct Parser {
    tokens: Vec<LocatedToken>,
    index: usize,
    stack: Vec<Box<dyn Error>>,
    registry: HashMap<String, ReturnType>,
}

#[cfg(test)]
mod tests {
    use super::*;
    const BASIC_OP_COUNT: u8 = 10;
    const TEST_INPUT: &str = "src/tests/";

    fn path(add: &str) -> String {
        format!("{}/{}.txt", TEST_INPUT, add)
    }

    fn parser_from(file: &str) -> Result<Parser> {
        Parser::new(&path(file))
    }

    pub fn parser_from_tree_raw(unlocated: Vec<Token>) -> Parser {
        Parser::from_tree(
            unlocated
                .iter()
                .map(|t| LocatedToken::new(t.clone(), Location::internal().unwrap()))
                .collect(),
        )
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
            Symbol::Remainder,
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
            Token::Number(None, Some(17.0)),
        ];
        for token in expected {
            assert_eq!(parser.next_token().unwrap().tok(), token);
        }
        assert!(parser.next_token().is_err());
    }

    #[test]
    fn expect_identifier() {
        let mut parser = parser_from("basic").unwrap();
        assert_eq!(parser.next_token().unwrap().tok(), Token::Let);
        assert_eq!(parser.expect_identifier().unwrap(), "age".to_string());
    }

    #[test]
    fn look_ahead() {
        let tokens = vec![Token::Let, Token::Newline, Token::Let];
        let mut parser = parser_from_tree_raw(tokens);
        assert_eq!(parser.look_ahead().unwrap().tok(), Token::Let);
        assert_eq!(parser.look_ahead().unwrap().tok(), Token::Let);
        assert!(parser.look_ahead().is_err());
    }

    #[test]
    fn push() {
        let tokens = vec![Token::Let, Token::When, Token::Newline, Token::Extern];
        let mut parser = parser_from_tree_raw(tokens);
        parser.push(-1);
        assert_eq!(parser.index, 0);
        parser.push(1);
        assert_eq!(parser.index, 1);
        parser.push(-1);
        assert_eq!(parser.index, 0);
    }

    #[test]
    fn back() {
        let tokens = vec![Token::Let, Token::When, Token::Newline, Token::Extern];
        let mut parser = parser_from_tree_raw(tokens);
        parser.back();
        assert_eq!(parser.index, 0);
        parser.push(3);
        parser.back();
        assert_eq!(parser.index, 2);
    }

    #[test]
    fn forward() {
        let tokens = vec![Token::Let, Token::When, Token::Newline, Token::Extern];
        let mut parser = parser_from_tree_raw(tokens);
        parser.forward();
        assert_eq!(parser.index, 1);
        parser.forward();
        assert_eq!(parser.index, 2);
    }

    #[test]
    fn last() {
        let tokens = vec![Token::Let, Token::When, Token::Newline, Token::Extern];
        let mut parser = parser_from_tree_raw(tokens);
        assert!(parser.last().is_err());
        parser.forward();
        assert_eq!(parser.last().unwrap().tok(), Token::Let);
        parser.forward();
        assert_eq!(parser.last().unwrap().tok(), Token::When);
        parser.forward();
        assert_eq!(parser.last().unwrap().tok(), Token::Newline);
        parser.forward();
        assert_eq!(parser.last().unwrap().tok(), Token::Extern);
    }

    #[test]
    fn parse_expr_or_err() {
        let tokens_err = vec![Token::Let, Token::When, Token::Newline, Token::Extern];
        let tokens_err = vec![Token::Let, Token::When, Token::Newline, Token::Extern];
        let mut parser = parser_from_tree_raw(tokens_err);
        assert!(parser.parse_expr_or_err().is_err());
    }
}

impl Parser {
    // Create a new `Parser` from the given source file
    pub fn new(src: &str) -> Result<Parser> {
        let file = BufReader::new(File::open(src)?);
        let mut lexer = Interpreter::new(file, src.to_string())?;

        Ok(Parser {
            tokens: lexer.pull()?,
            index: 0,
            stack: vec![],
            registry: HashMap::new()
        })
    }

    pub fn from_tree(tokens: Vec<LocatedToken>) -> Parser {
        Parser {
            tokens,
            index: 0,
            stack: vec![],
            registry: HashMap::new()
        }
    }

    pub fn or_dump<T>(&self, opt: Result<T>) -> T {
        match opt {
            Ok(v) => v,
            Err(e) => {
                self.print_error();
                panic!("{}", e);
            }
        }
    }

    fn add_to_stack(&mut self, error: Box<dyn Error>) {
        self.stack.push(error);
    }

    pub fn print_error(&self) {
        for error in &self.stack {
            panic!("{}", *error);
        }
    }

    // if you change this, make sure to change BASIC_OP_COUNT above
    fn basic_op_prec(symbol: Symbol) -> Option<u8> {
        match symbol {
            Symbol::ToPower => Some(0),
            Symbol::Multiply => Some(1),
            Symbol::Divide => Some(2),
            Symbol::Remainder => Some(3),
            Symbol::Add => Some(4),
            Symbol::Subtract => Some(5),
            Symbol::LessThan => Some(6),
            Symbol::GreaterThan => Some(7),
            Symbol::Comma => Some(8),
            Symbol::Equals => Some(9),
            _ => None,
        }
    }

    fn compound_op_prec(symbols: (Symbol, Symbol)) -> Option<u8> {
        match symbols {
            (Symbol::LessThan, Symbol::Equals) => Some(10),
            (Symbol::GreaterThan, Symbol::Equals) => Some(11),
            (Symbol::Equals, Symbol::Equals) => Some(12),
            (Symbol::Negate, Symbol::Equals) => Some(13),
            (Symbol::Ampersand, Symbol::Ampersand) => Some(14),
            (Symbol::Pipe, Symbol::Pipe) => Some(15),
            _ => None,
        }
    }

    fn last(&self) -> Result<LocatedToken> {
        if self.index == 0 || self.tokens.len() < self.index - 1 {
            EofError::while_initializing()
        } else {
            Ok(self.tokens[self.index - 1].clone())
        }
    }

    fn next_token(&mut self) -> Result<LocatedToken> {
        if self.index >= self.tokens.len() {
            self.add_to_stack(EofError::basic(Location::internal()).into());
            EofError::while_initializing()
        } else {
            let next = self.tokens[self.index].clone();
            self.forward();
            Ok(next)
        }
    }

    fn expect_identifier(&mut self) -> Result<String> {
        let token = self.look_ahead()?;
        if let Token::Identifier(identifier) = token.tok() {
            Ok(identifier)
        } else {
            IdentifierError::while_parsing(token.loc())
        }
    }

    // no coverage
    fn parse_expr_or_err(&mut self) -> Result<LocatedExpr> {
        match self.parse_expression() {
            Some(expr) => Ok(expr),
            _ => DeclarationError::while_parsing(self.last()?.loc()),
        }
    }

    fn trace_return_type(&mut self, expr: &LocatedExpr) -> Result<ReturnType> {
        match expr.get_expr() {
            Expr::NumberLiteral(floating, _, _) => Ok(if floating {
                ReturnType::Float
            } else {
                ReturnType::Int
            }),
            Expr::BinaryOperation(_, left, right) => {
                let left_type = self.trace_return_type(&left)?;
                let right_type = self.trace_return_type(&right)?;
                if left_type == right_type {
                    Ok(left_type)
                } else {
                    UnbalancedBinaryExpressionError::while_parsing(right.get_location())
                }
            }
            Expr::StringLiteral(_) => Ok(ReturnType::String),
            Expr::FunctionDefinition(_, _, body, _) => self.trace_return_type(&body),
            Expr::ChainExpression(links) => {
                let mut last_link_type = None;
                for link in links {
                    let link_type = self.trace_return_type(&link)?;
                    if last_link_type != None && last_link_type != Some(link_type.clone()) {
                        return UnbalancedChainExpressionError::while_parsing(link.get_location());
                    } else {
                        last_link_type = Some(link_type);
                    }
                }
                Ok(last_link_type.unwrap())
            }
            Expr::WhenExpression(_, result) => self.trace_return_type(&result),
            Expr::FunctionCall(name, _) => {
                Ok(self.registry.get(&name).unwrap().clone())
            },
            Expr::VariableRef(_) => {
                
            },
        }
    }

    fn parse_definition(&mut self) -> Result<LocatedExpr> {
        let identifier = self.expect_identifier()?;
        let next = self.next_token()?;
        match next.tok() {
            Token::Grouping(args) => {
                let parsed_args = Parser::parse_def_args(args)?;
                let body = self.parse_expr_or_err()?.into();
                let return_type = self.trace_return_type(&body)?;
                self.registry.insert(identifier.clone(), return_type.clone());
                Ok(LocatedExpr::new(
                    Expr::FunctionDefinition(identifier, parsed_args, body.into(), return_type),
                    next.loc(),
                ))
            }
            _ => DeclarationError::while_parsing(next.loc()),
        }
    }

    fn look_ahead(&mut self) -> Result<LocatedToken> {
        loop {
            let token = self.next_token()?;
            match token.tok() {
                Token::Newline => {}
                _ => {
                    return Ok(token);
                }
            }
        }
    }

    fn look_behind(&mut self) -> Result<LocatedToken> {
        loop {
            self.push(-2);
            let token = self.next_token()?;
            match token.tok() {
                Token::Newline => {}
                _ => {
                    self.forward();
                    return Ok(token);
                }
            }
        }
    }

    fn behind_is_expr(&mut self) -> Option<LocatedToken> {
        if let Ok(behind) = self.look_behind() {
            match behind.tok() {
                Token::Identifier(_) | Token::Grouping(_) => Some(behind),
                _ => None,
            }
        } else {
            None
        }
    }

    fn behind_is_operation(&mut self) -> Option<LocatedToken> {
        if let Ok(behind) = self.look_behind() {
            match behind.tok() {
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

    fn read_expr_tokens(&mut self) -> Vec<LocatedToken> {
        let mut tokens = vec![];
        loop {
            let Ok(token) = self.next_token() else { break };
            match token.tok() {
                Token::Newline => {
                    let Ok(next) = self.next_token() else { break };
                    if let Token::Let = next.tok() {
                        self.back();
                        break;
                    } else if let Token::Symbol(_) = next.tok() {
                        if self.behind_is_expr().is_none() {
                            break;
                        }
                    } else if let Token::When = next.tok() {
                        self.back();
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
                    if tokens.len() > 0 || !matches!(token.tok(), Token::Symbol(Symbol::Equals)) {
                        tokens.push(token);
                    }
                }
                _ => {
                    break;
                }
            }
        }
        return tokens;
    }

    fn parse_grouping(tokens: Vec<LocatedToken>, with_brackets: bool) -> Option<LocatedExpr> {
        let change = with_brackets as usize;

        if tokens.len() > 2 * change {
            let mut parser = Parser::from_tree(tokens[change..tokens.len() - change].to_vec());
            Some(parser.parse_expression()?)
        } else {
            None
        }
    }

    fn parse_non_symbol(&mut self, symbol: LocatedToken) -> Option<LocatedExpr> {
        match symbol.tok() {
            Token::Identifier(ident) => {
                Some(LocatedExpr::new(Expr::VariableRef(ident), symbol.loc()))
            }
            Token::Number(int, float) => Some(LocatedExpr::new(
                Expr::NumberLiteral(int.is_none(), int.unwrap_or(0), float.unwrap_or(0.0)),
                symbol.loc(),
            )),
            Token::String(s) => Some(LocatedExpr::new(Expr::StringLiteral(s), symbol.loc())),
            Token::Grouping(tokens) => Parser::parse_grouping(tokens, true),
            // fix
            Token::FunctionCall(name, tokens) => {
                let call = if let Ok(args) = Parser::parse_chain(tokens, false) {
                    Expr::FunctionCall(name, args)
                } else {
                    Expr::FunctionCall(name, vec![])
                };
                Some(LocatedExpr::new(call, symbol.loc()))
            }
            _ => None,
        }
    }

    fn branch(
        &mut self,
        tokens: Vec<LocatedToken>,
        mut indexes: Vec<(usize, u8)>,
    ) -> Option<LocatedExpr> {
        let mut last_index = 0;
        let mut tree = None;
        indexes.sort_by(|a, b| a.1.cmp(&b.1));

        for (index, prec) in indexes {
            let max = tokens.len();
            if index >= max - 1 {
                self.add_to_stack(
                    UnbalancedBinaryExpressionError::basic(Some(tokens[index].loc())).into(),
                );
                return None;
            }

            match tree {
                None => {
                    let branch = Expr::BinaryOperation(
                        prec,
                        self.parse_non_symbol(tokens[index - 1].clone())?.into(),
                        self.parse_non_symbol(tokens[index + 1].clone())?.into(),
                    );
                    tree = Some(LocatedExpr::new(branch, tokens[index].loc()));
                    last_index = index;
                }
                Some(t) => {
                    let branch;
                    if last_index < index {
                        branch = Expr::BinaryOperation(
                            prec,
                            t.into(),
                            self.parse_non_symbol(tokens[index + 1].clone())?.into(),
                        );
                    } else {
                        branch = Expr::BinaryOperation(
                            prec,
                            self.parse_non_symbol(tokens[index - 1].clone())?.into(),
                            t.into(),
                        );
                    }
                    tree = Some(LocatedExpr::new(branch, tokens[index].loc()));
                    last_index = index;
                }
            }
        }

        match tree {
            None => Some(self.parse_non_symbol(tokens[0].clone())?),
            t => t,
        }
    }

    fn parse_expression(&mut self) -> Option<LocatedExpr> {
        let tokens = self.read_expr_tokens();
        if tokens.len() == 0 {
            return None;
        }

        for token in &tokens {
            if token.tok() == Token::Symbol(Symbol::Comma) {
                let expressions = Parser::parse_chain(tokens.clone(), false);
                return match expressions {
                    Err(e) => {
                        self.add_to_stack(e);
                        None
                    }
                    Ok(expr) => Some(LocatedExpr::new(Expr::ChainExpression(expr), token.loc())),
                };
            }
        }

        let mut reader = Parser::from_tree(tokens.clone());
        let mut op_indexes = vec![];
        let mut when_index = 0;

        loop {
            let Ok(next) = reader.next_token() else { break };
            match next.tok() {
                Token::When => {
                    when_index = reader.index - 1;
                }
                Token::Symbol(Symbol::Compound(first, second)) => {
                    op_indexes.push((
                        reader.index - 1,
                        Parser::compound_op_prec((*first, *second))?,
                    ));
                }
                // auto unwrap because otherwise it would not have even been added to token list
                Token::Symbol(op) => {
                    op_indexes.push((reader.index - 1, Parser::basic_op_prec(op)?))
                }
                _ => {}
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

            return Some(LocatedExpr::new(
                Expr::WhenExpression(
                    self.branch(tokens[when_index + 1..].to_vec(), right)?
                        .into(),
                    self.branch(tokens[..when_index].to_vec(), left)?.into(),
                ),
                tokens[when_index].loc(),
            ));
        } else {
            self.branch(tokens, op_indexes)
        }
    }

    fn parse_def_args(args: Vec<LocatedToken>) -> Result<Vec<String>> {
        let mut arg_tree = vec![];
        let mut p = Parser::from_tree(args.clone());

        p.forward();
        while let Ok(identifier) = p.expect_identifier() {
            if let Ok(next) = p.next_token() {
                match next.tok() {
                    Token::Symbol(Symbol::Comma) => {
                        arg_tree.push(identifier);
                    }
                    Token::Bracket(Bracket::Parens(Is::Closed)) => {
                        arg_tree.push(identifier);
                        break;
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
        return Ok(arg_tree);
    }

    fn parse_one_of_chain(current_group: Vec<LocatedToken>) -> Result<LocatedExpr> {
        if let Some(parsed) = Parser::parse_grouping(current_group.clone(), false) {
            Ok(parsed)
        } else {
            BadCommaError::while_parsing(current_group.last().unwrap().loc())
        }
    }

    fn parse_chain(args: Vec<LocatedToken>, is_arg: bool) -> Result<Vec<LocatedExpr>> {
        let mut arg_tree = vec![];
        let mut p = Parser::from_tree(args);
        let mut current_group = vec![];
        if is_arg {
            p.forward();
        }

        while let Ok(next) = p.next_token() {
            match next.tok() {
                Token::Symbol(Symbol::Comma) | Token::Bracket(Bracket::Parens(Is::Closed)) => {
                    let parsed = Parser::parse_one_of_chain(current_group)?;
                    arg_tree.push(parsed);
                    current_group = vec![];

                    // really fucking interesting
                    // just revisited this. holy shit
                    if let Token::Bracket(Bracket::Parens(Is::Closed)) = next.tok() {
                        return Ok(arg_tree);
                    }
                }
                _ => {
                    current_group.push(next);
                }
            }
        }

        if is_arg {
            BadCommaError::while_parsing(current_group.last().unwrap().loc())
        } else {
            arg_tree.push(Parser::parse_one_of_chain(current_group)?);
            Ok(arg_tree)
        }
    }

    pub fn run(&mut self) -> Vec<LocatedExpr> {
        let mut tree: Vec<LocatedExpr> = vec![];

        loop {
            let Ok(token) = self.next_token() else { break };

            if let Token::Let = token.tok() {
                match self.parse_definition() {
                    Ok(def) => {
                        tree.push(def);
                    }
                    Err(e) => {
                        self.print_error();
                        panic!("{}", e);
                    }
                }
            } else {
                match token.tok() {
                    Token::Identifier(_)
                    | Token::Extern
                    | Token::Grouping(_)
                    | Token::FunctionCall(_, _) => {
                        self.back();
                        let expr = self.parse_expr_or_err();
                        tree.push(self.or_dump(expr));
                    }
                    _ => {}
                }
            }
        }

        return tree;
    }
}
