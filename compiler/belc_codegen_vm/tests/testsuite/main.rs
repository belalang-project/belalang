use std::error::Error;

use belc_ast::Parser;
use belc_codegen_vm::Compiler;
use belc_lexer::Lexer;
use belvm_bytecode::opcode;
use belvm_bytecode::{Bytecode, Constant};

#[track_caller]
fn test_compile(input: &str) -> Result<Bytecode, Box<dyn Error>> {
    let source = input.to_owned();
    let lexer = Lexer::new(&source);
    let mut parser = Parser::new(lexer);
    let program = parser.parse_program()?;

    let mut compiler = Compiler::default();
    let code = compiler.compile_program(program)?;

    Ok(code)
}

#[test]
fn integer_literals() {
    let code = test_compile("1; 2; 3;").unwrap();

    #[rustfmt::skip]
    assert_eq!(code.instructions, vec![
        opcode::CONSTANT, 0, 0,
        opcode::POP,
        opcode::CONSTANT, 0, 1,
        opcode::POP,
        opcode::CONSTANT, 0, 2,
        opcode::POP,
        opcode::RETURN_VALUE,
    ]);

    assert_eq!(
        code.constants,
        vec![Constant::Integer(1), Constant::Integer(2), Constant::Integer(3),]
    );
}

#[test]
fn booleans() {
    let code = test_compile("true; false;").unwrap();

    #[rustfmt::skip]
    assert_eq!(code.instructions, vec![
        opcode::TRUE,
        opcode::POP,
        opcode::FALSE,
        opcode::POP,
        opcode::RETURN_VALUE,
    ]);

    assert_eq!(code.constants, vec![]);
}

#[track_caller]
fn test_compile_infix(op: &str, code: u8, reversed: bool) {
    let input = format!("1 {op} 3;");
    let compiled = test_compile(&input).unwrap();

    #[rustfmt::skip]
    assert_eq!(compiled.instructions, vec![
        opcode::CONSTANT, 0, 0,
        opcode::CONSTANT, 0, 1,
        code,
        opcode::POP,
        opcode::RETURN_VALUE,
    ]);

    assert_eq!(
        compiled.constants,
        if reversed {
            vec![Constant::Integer(3), Constant::Integer(1)]
        } else {
            vec![Constant::Integer(1), Constant::Integer(3)]
        }
    );
}

#[test]
fn infix_expressions() {
    test_compile_infix("+", opcode::ADD, false);
    test_compile_infix("-", opcode::SUB, false);
    test_compile_infix("*", opcode::MUL, false);
    test_compile_infix("/", opcode::DIV, false);
    test_compile_infix("%", opcode::MOD, false);
    test_compile_infix("==", opcode::EQUAL, false);
    test_compile_infix("!=", opcode::NOT_EQUAL, false);
    test_compile_infix("<", opcode::LESS_THAN, false);
    test_compile_infix("<=", opcode::LESS_THAN_EQUAL, false);
    test_compile_infix(">", opcode::LESS_THAN, true);
    test_compile_infix(">=", opcode::LESS_THAN_EQUAL, true);
}

#[test]
fn prefix_expressions() {
    let code = test_compile("-5;").unwrap();

    #[rustfmt::skip]
    assert_eq!(code.instructions, vec![
        opcode::CONSTANT, 0, 0,
        opcode::MINUS,
        opcode::POP,
        opcode::RETURN_VALUE,
    ]);

    assert_eq!(code.constants, vec![Constant::Integer(5),]);
}

#[test]
fn if_expressions() {
    let code = test_compile("if (1 == 1) { 10 }; 9;").unwrap();

    #[rustfmt::skip]
    assert_eq!(code.instructions, vec![
        opcode::CONSTANT, 0, 0,
        opcode::CONSTANT, 0, 1,
        opcode::EQUAL,
        opcode::JUMP_IF_FALSE, 0, 6,
        opcode::CONSTANT, 0, 2,
        opcode::JUMP, 0, 1,
        opcode::NULL,
        opcode::POP,
        opcode::CONSTANT, 0, 3,
        opcode::POP,
        opcode::RETURN_VALUE,
    ]);

    assert_eq!(
        code.constants,
        vec![
            Constant::Integer(1),
            Constant::Integer(1),
            Constant::Integer(10),
            Constant::Integer(9),
        ]
    );
}

#[test]
fn if_else_expressions() {
    let code = test_compile("if (true) { 10 } else { 11 };").unwrap();

    #[rustfmt::skip]
    assert_eq!(code.instructions, vec![
        opcode::TRUE,
        opcode::JUMP_IF_FALSE, 0, 6,
        opcode::CONSTANT, 0, 0,
        opcode::JUMP, 0, 3,
        opcode::CONSTANT, 0, 1,
        opcode::POP,
        opcode::RETURN_VALUE,
    ]);

    assert_eq!(code.constants, vec![Constant::Integer(10), Constant::Integer(11),]);
}

#[test]
fn if_else_if_expressions() {
    let code = test_compile(
        "
        if (true) { 10 }
        else if (true) { 11 }
        else { 12 };
    ",
    )
    .unwrap();

    #[rustfmt::skip]
    assert_eq!(code.instructions, vec![
        opcode::TRUE,
        opcode::JUMP_IF_FALSE, 0, 6,
        opcode::CONSTANT, 0, 0,
        opcode::JUMP, 0, 13,
        opcode::TRUE,
        opcode::JUMP_IF_FALSE, 0, 6,
        opcode::CONSTANT, 0, 1,
        opcode::JUMP, 0, 3,
        opcode::CONSTANT, 0, 2,
        opcode::POP,
        opcode::RETURN_VALUE,
    ]);

    assert_eq!(
        code.constants,
        vec![Constant::Integer(10), Constant::Integer(11), Constant::Integer(12),]
    );
}

#[test]
fn var() {
    let code = test_compile("x := 12; x = 11; x;").unwrap();

    #[rustfmt::skip]
    assert_eq!(code.instructions, vec![
        opcode::CONSTANT, 0, 0,
        opcode::SET_GLOBAL, 0, 1,
        opcode::POP,
        opcode::CONSTANT, 0, 1,
        opcode::SET_GLOBAL, 0, 1,
        opcode::POP,
        opcode::GET_GLOBAL, 0, 1,
        opcode::POP,
        opcode::RETURN_VALUE,
    ]);

    assert_eq!(code.constants, vec![Constant::Integer(12), Constant::Integer(11),]);
}

#[test]
fn var_assignment_ops() {
    let code = test_compile("x := 1; x += 1;").unwrap();

    #[rustfmt::skip]
    assert_eq!(code.instructions, vec![
        opcode::CONSTANT, 0, 0,
        opcode::SET_GLOBAL, 0, 1,
        opcode::POP,
        opcode::GET_GLOBAL, 0, 1,
        opcode::CONSTANT, 0, 1,
        opcode::ADD,
        opcode::SET_GLOBAL, 0, 1,
        opcode::POP,
        opcode::RETURN_VALUE,
    ]);

    assert_eq!(code.constants, vec![Constant::Integer(1), Constant::Integer(1),]);
}

#[test]
fn block_expression() {
    let code = test_compile("{ x := 12; };").unwrap();

    #[rustfmt::skip]
    assert_eq!(code.instructions, vec![
        opcode::CONSTANT, 0, 0,
        opcode::SET_GLOBAL, 0, 1, // need to change when block scope
        opcode::POP,
        opcode::RETURN_VALUE,
    ]);

    assert_eq!(code.constants, vec![Constant::Integer(12),]);
}

// #[test]
// fn function_expressions() {
//     let code = test_compile("ten := fn() { 10 };").unwrap();
//
//     assert_eq!(code.instructions, vec![
//         opcode::CONSTANT, 0, 1,
//         opcode::SET_GLOBAL, 0, 1,
//         opcode::POP,
//         opcode::RETURN_VALUE,
//
//         // ten function instructions
//         opcode::CONSTANT, 0, 0,
//         opcode::RETURN_VALUE,
//     ]);
//
//     assert_eq!(code.constants, vec![
//         Constant::Integer(10),
//         Constant::Function(Function {
//             pointer: 8,
//             locals_count: 0,
//             arity: 0
//         })
//     ]);
// }
//
// #[test]
// fn function_with_args_expressions() {
//     let code = test_compile("add := fn(a, b) { a + b }; three := add(1,
// 2);").unwrap();
//
//     assert_eq!(code.instructions, vec![
//         opcode::CONSTANT, 0, 0,
//         opcode::SET_GLOBAL, 0, 1,
//         opcode::POP,
//         opcode::CONSTANT, 0, 1,
//         opcode::CONSTANT, 0, 2,
//         opcode::GET_GLOBAL, 0, 1,
//         opcode::CALL,
//         opcode::SET_GLOBAL, 0, 2,
//         opcode::POP,
//         opcode::RETURN_VALUE,
//
//         // add function instructions
//         opcode::GET_LOCAL, 0,
//         opcode::GET_LOCAL, 1,
//         opcode::ADD,
//         opcode::RETURN_VALUE,
//     ]);
//
//     assert_eq!(code.constants, vec![
//         Constant::Function(Function {
//             pointer: 22,
//             locals_count: 2,
//             arity: 2,
//         }),
//         Constant::Integer(2),
//         Constant::Integer(1),
//     ]);
// }
