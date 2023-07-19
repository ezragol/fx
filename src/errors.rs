use std::{error::Error, fmt};

pub type Result<T> = std::result::Result<T, Box<dyn Error>>;

macro_rules! def {
    ($name:ident, $error:expr) => {
        #[derive(Debug, Clone)]
        pub struct $name;
        impl Error for $name {}

        impl fmt::Display for $name {
            fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
                write!(f, $error)
            }
        }
    };
}

def!(DeclarationError, "expected declaration!");
def!(RangeError, "improperly formatted range!");
def!(EofError, "unexpected end of file!");
def!(IdentifierError, "expected identifier!");
def!(GroupingError, "improperly formatted grouping!");
def!(UnknownTokenError, "unknown token!");