#![feature(assert_matches)]

use belc_ast::Parser;
use belc_codegen_vm::Compiler;
use belc_lexer::Lexer;
use belvm_bytecode::Bytecode;

pub mod error;

pub fn compile(source: &String) -> Bytecode {
    let lexer = Lexer::new(source);
    let mut parser = Parser::new(lexer);
    let program = parser.parse_program().unwrap();

    let mut compiler = Compiler::default();
    compiler.compile_program(program).unwrap()
}
