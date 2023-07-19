use crate::parser::Parser;
use std::time;

#[test]
fn parser() {
    let now = time::Instant::now();
    let parser = Parser::new("test.txt");
    match parser {
        Ok(mut p) => p.run(),
        Err(e) => err!("{}", e)
    }

    println!("{:?}", now.elapsed());
}
