use crate::parser::Parser;
use std::time;

#[test]
fn parser() {
    let now = time::Instant::now();
    let mut parser = Parser::new("test.txt").unwrap();
    parser.run();

    println!("{:?}", now.elapsed());
}
