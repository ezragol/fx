use crate::errors::{BadArgumentError, MissingOutputFileError, Result};
use std::ffi::{c_char, CStr};

#[derive(Debug)]
pub struct Options {
    pub filename: String,
    pub outfile: String,
    pub original_args: Vec<String>,
}

impl Options {
    pub fn new(argv_ptr: *mut *mut c_char, size: usize) -> Result<Options> {
        let mut argv: Vec<String> = vec![];
        unsafe {
            let slice = std::slice::from_raw_parts(argv_ptr, size);
            for string in slice {
                match CStr::from_ptr(string.clone()).to_str() {
                    Ok(s) => argv.push(s.to_string()),
                    Err(_) => return BadArgumentError::while_initializing(),
                }
            }
        }

        if argv.len() > 2 {
            Ok(Options {
                filename: argv[1].clone(),
                outfile: argv[2].clone(),
                original_args: argv,
            })
        } else {
            MissingOutputFileError::while_initializing()
        }
    }
}
