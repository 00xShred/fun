mod ast;
mod interpreter;
mod lexer;
mod parser;
mod token;

use std::io;

use interpreter::Interpreter;
use lexer::Lexer;
use parser::Parser;

fn main() {
    println!("Enter an expression:");

    let mut source = String::new();
    io::stdin()
        .read_line(&mut source)
        .expect("failed to read input");

    let source = source.trim();
    if source.is_empty() {
        eprintln!("no input provided");
        return;
    }

    let source = format!("{source};");

    let mut lexer = Lexer::new(&source);
    let tokens = lexer.tokenize().expect("lexing failed");

    let mut parser = Parser::new(tokens);
    let program = parser.parse_program().expect("parsing failed");

    let mut interpreter = Interpreter::new();
    let result = interpreter.run(&program).expect("runtime error");
    println!("result: {result:?}");
}
