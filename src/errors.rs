use std::{fmt, error::Error};

pub type Result<T> = std::result::Result<T, Box<dyn Error>>;

#[derive(Debug, Clone)]
pub struct DeclarationError;

#[derive(Debug, Clone)]
pub struct RangeError;

#[derive(Debug, Clone)]
pub struct EofError;

#[derive(Debug, Clone)]
pub struct IdentifierError;

#[derive(Debug, Clone)]
pub struct UnexpectedTokenError<T, U> {
    expected: T,
    actual: U
}

impl <T, U> UnexpectedTokenError<T, U> {
    pub fn new(expected: T, actual: U) -> UnexpectedTokenError<T, U> {
        UnexpectedTokenError { expected, actual }
    }
}

impl fmt::Display for DeclarationError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "expected declaration!")
    }
}

impl fmt::Display for RangeError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "expected properly formatted range!")
    }
}

impl fmt::Display for EofError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "unexpected end of file!")
    }
}

impl fmt::Display for IdentifierError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "expected identifier!")
    }
}

impl <T, U> fmt::Display for UnexpectedTokenError<T, U>
    where T: fmt::Debug, U: fmt::Debug {

    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "expected type {:#?}, found {:#?}!", self.expected, self.actual)
    }
}

impl Error for DeclarationError {}
impl Error for RangeError {} 
impl Error for EofError {}
impl Error for IdentifierError {}
impl <T: fmt::Debug, U: fmt::Debug> Error for UnexpectedTokenError<T, U> {}