use std::{error::Error, fmt};

pub type Result<T> = std::result::Result<T, Box<dyn Error>>;

#[derive(Clone, Debug, PartialEq)]
pub struct Location {
    line: usize,
    column: usize,
    filename: String,
    last_line_width: usize
}

impl Location {
    pub fn new(line: usize, column: usize, filename: String) -> Location {
        Location {
            line,
            column,
            filename,
            last_line_width: 0
        }
    }

    pub fn internal() -> Option<Location> {
        Some(Location {
            line: 0,
            column: 0,
            filename: "interal".to_string(),
            last_line_width: 0
        })
    }

    pub fn get_line(&self) -> usize {
        self.line
    }

    pub fn get_column(&self) -> usize {
        self.column
    }

    pub fn get_filename(&self) -> String {
        self.filename.clone()
    }

    pub fn next_line(&mut self) {
        self.last_line_width = self.column;
        self.column = 0;
        self.line += 1;
    }

    pub fn previous_column(&mut self) {
        if self.column == 0 {
            self.line -= 1;
            self.column = self.last_line_width;
        }
        self.column -= 1;
    }

    pub fn next_column(&mut self) {
        self.column += 1;
    }

    pub fn get_message(&self) -> String {
        format!("@{}:{}:{}", self.filename, self.line + 1, self.column + 1)
    }
}

macro_rules! def {
    ($name:ident, $error:expr) => {
        #[derive(Debug, Clone)]
        pub struct $name {
            location: Option<Location>,
            stage_name: String,
        }
        impl $name {
            #[allow(dead_code)]
            pub fn new<T>(location: Option<Location>, stage_name: &str) -> Result<T> {
                Err(($name {
                    location,
                    stage_name: stage_name.to_string(),
                })
                .into())
            }

            #[allow(dead_code)]
            pub fn basic(location: Option<Location>) -> $name {
                $name {
                    location,
                    stage_name: "parse".to_string(),
                }
            }

            #[allow(dead_code)]
            pub fn while_parsing<T>(location: Location) -> Result<T> {
                $name::new(Some(location), "parse")
            }

            #[allow(dead_code)]
            pub fn while_initializing<T>() -> Result<T> {
                $name::new(None, "init")
            }
        }

        impl Error for $name {}

        impl fmt::Display for $name {
            fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
                let msg = if let Some(l) = self.location.clone() {
                    l.get_message()
                } else {
                    "internal".to_string()
                };
                write!(
                    f,
                    "\n\nERROR: [{}]\n >>   {}, {}\n\n",
                    self.stage_name, msg, $error
                )
            }
        }
    };
}

def!(DeclarationError, "expected function declaration!");
def!(RangeError, "improperly formatted range!");
def!(EofError, "unexpected end of file!");
def!(IdentifierError, "expected identifier!");
def!(GroupingError, "improperly formatted grouping!");
def!(UnknownTokenError, "unknown token!");
def!(BadCommaError, "comma placed badly!");
def!(BadArgumentError, "bad argument...");
def!(MissingOutputFileError, "no output file supplied!");
def!(
    UnbalancedBinaryExpressionError,
    "unbalanced binary expression!"
);
def!(
    UnbalancedChainExpressionError,
    "chain does not have consistent types!"
);
