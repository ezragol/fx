pub trait Expression: std::fmt::Debug {}

pub type Expr = Box<dyn Expression>;

#[derive(Debug)]
pub struct NumberLiteral {
    floating: bool,
    int_val: isize,
    float_val: f64,
}

#[derive(Debug)]
pub struct VariableDefinition {
    name: String,
    value: Expr,
}

#[derive(Debug)]
pub struct FunctionDefinition {
    name: String,
    args: Vec<String>,
    body: Expr,
}

#[derive(Debug)]
pub struct ChainExpression {
    expressions: Vec<Expr>,
}

#[derive(Debug)]
pub struct BinaryOperation {
    op: u8,
    left: Expr,
    right: Expr,
}

#[derive(Debug)]
pub struct WhenExpression {
    result: Expr,
    predicate: BinaryOperation,
}

#[derive(Debug)]
pub struct Range {
    start: Expr,
    end: Expr,
}

#[derive(Debug)]
pub struct FunctionCall {
    name: String,
    args: Vec<Expr>,
}

#[derive(Debug)]
pub struct VariableRef {
    name: String
}

impl NumberLiteral {
    pub fn new(floating: bool, int_val: isize, float_val: f64) -> NumberLiteral {
        NumberLiteral {
            floating,
            int_val,
            float_val,
        }
    }
}

impl VariableDefinition {
    pub fn new(name: String, value: Expr) -> VariableDefinition {
        VariableDefinition { name, value }
    }
}

impl FunctionDefinition {
    pub fn new(name: String, args: Vec<String>, body: Expr) -> FunctionDefinition {
        FunctionDefinition { name, args, body }
    }
}

impl Range {
    pub fn new(start: Expr, end: Expr) -> Range {
        Range { start, end }
    }
}

impl BinaryOperation {
    pub fn new(op: u8, left: Expr, right: Expr) -> BinaryOperation {
        BinaryOperation { op, left, right }
    }
}

impl VariableRef {
    pub fn new(name: String) -> VariableRef {
        VariableRef { name }
    }
}

impl Expression for NumberLiteral {}
impl Expression for Range {}
impl Expression for VariableDefinition {}
impl Expression for FunctionDefinition {}
impl Expression for BinaryOperation {}
impl Expression for VariableRef {}