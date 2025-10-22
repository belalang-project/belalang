use std::{
    error::Error,
    io::{
        self,
        Write,
    },
};

use belc_lexer::{
    Lexer,
    TokenKind,
};

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

        let mut lexer = Lexer::new(&input);

        loop {
            match lexer.next_token() {
                Ok(token) if token.kind == TokenKind::EOF => break,
                Ok(token) => println!("{token:?}"),
                Err(err) => println!("ERROR: {err}"),
            };
        }

        println!();
    }
}
