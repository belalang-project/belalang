use std::{
    error::Error,
    io::{
        self,
        Write,
    },
};

use belc_ast::Parser;
use belc_lexer::Lexer;

fn main() -> Result<(), Box<dyn Error>> {
    let mut input = String::new();

    loop {
        print!(">> ");
        io::stdout().flush().unwrap();

        input.clear();
        io::stdin().read_line(&mut input).unwrap();

        if input.trim().is_empty() {
            println!();
            continue;
        }

        let lexer = Lexer::new(&input);
        let mut parser = Parser::new(lexer);

        match parser.parse_program() {
            Ok(program) => println!("{:#?}", program.statements),
            Err(err) => println!("ERROR: {err}"),
        }

        println!();
    }
}
