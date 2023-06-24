pub trait Expression {
    fn codegen(&self) -> i32 {
        0
    }
}

pub struct NumberExpression {
    value: f64,
}

pub struct VariableReference {
    name: &'static str,
}

pub struct BinaryOperation {
    operation: ,
    left: Box<dyn Expression>,
    right: Box<dyn Expression>,
}

pub struct FunctionCall {
    name: &'static str,
    args: Vec<Box<dyn Expression>>,
}

pub struct FunctionDefinition {
    name: &'static str,
    args: Vec<&'static str>,
    body: dyn Expression,
}

impl NumberExpression {
    pub fn new(value: f64) -> NumberExpression {
        NumberExpression { value }
    }
}

impl Expression for NumberExpression {
    fn codegen(&self) -> i32 {
        0
    }
}
