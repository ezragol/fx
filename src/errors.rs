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
pub struct UnexpectedTokenError<T> {
    expected: T,
}

impl <T> UnexpectedTokenError<T> {
    pub fn new(expected: T) -> UnexpectedTokenError<T> {
        UnexpectedTokenError { expected }
    }
}

impl fmt::Display for DeclarationError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "expected declaration!")
    }
}

impl fmt::Display for RangeError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "improperly formatted range!")
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

impl <T> fmt::Display for UnexpectedTokenError<T>
    where T: fmt::Debug {

    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "expected {:?}!", self.expected)
    }
}

impl Error for DeclarationError {}
impl Error for RangeError {} 
impl Error for EofError {}
impl Error for IdentifierError {}
impl <T: fmt::Debug> Error for UnexpectedTokenError<T> {}