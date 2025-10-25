use std::{
    error::Error,
    io::{
        self,
        Write,
    },
};

use belc_ast::Parser;
use belc_codegen_vm::Compiler;
use belc_lexer::Lexer;
use belvm_bytecode::dis::disassemble;

fn compile(line: String) -> Result<(), Box<dyn Error>> {
    let lexer = Lexer::new(&line);
    let mut parser = Parser::new(lexer);
    let program = parser.parse_program()?;

    let mut compiler = Compiler::default();
    let code = compiler.compile_program(program)?;

    let disassembled = disassemble(code.instructions);

    println!("===== BYTECODE =====");
    print!("{disassembled}");

    println!("\n===== CONSTANTS =====");
    for (i, constant) in code.constants.iter().enumerate() {
        println!("{i:#04x}: {constant:?}");
    }

    println!("\n===== SYMBOLS =====");
    println!("{:#?}", compiler.scope.current().symbol_store);

    println!();

    Ok(())
}

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

        if let Err(error) = compile(input.clone()) {
            println!("ERROR: {error}");
        }
    }
}
