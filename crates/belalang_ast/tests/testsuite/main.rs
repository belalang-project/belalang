mod expressions;
mod statements;

pub mod common {
    use belalang_ast as ast;
    use belalang_ast::Parser;
    use belalang_lexer::Lexer;

    #[track_caller]
    pub fn test_parse(input: &str) -> ast::Program {
        let source = input.to_owned();
        let lexer = Lexer::new(&source);
        let mut parser = Parser::new(lexer);

        parser.parse_program().expect("parser errors")
    }

    #[track_caller]
    pub fn test_parse_to_string(input: &str, expected: &str) {
        let program = test_parse(input);
        assert_eq!(program.to_string(), expected);
    }

    #[macro_export]
    macro_rules! as_variant {
        ($value:expr, $variant:path) => {
            if let $variant(x) = $value {
                x
            } else {
                panic!("unmatching variant! got={}", stringify!($variant));
            }
        };
    }

    #[macro_export]
    macro_rules! ident_has_name {
        ($value:expr, $expected:expr) => {
            assert_eq!($value.value, $expected);
        };
    }

    #[macro_export]
    macro_rules! expr_variant {
        ($value:expr, $variant:path = $expected:expr) => {
            let v = as_variant!($value, $variant);

            assert_eq!(v.value, $expected);
        };
        ($value:expr, Infix => ($left_variant:path = $left:expr, $op:expr, $right_variant:path = $right:expr)) => {
            let v = as_variant!($value, ast::Expression::Infix);

            expr_variant!(&*v.left, $left_variant = $left);
            expr_variant!(&*v.right, $right_variant = $right);
            assert_eq!(v.operator, $op);
        };
        ($value: expr, Prefix => ($op:expr, $right_variant:path = $right:expr)) => {
            let v = as_variant!($value, ast::Expression::Prefix);

            expr_variant!(&*v.right, $right_variant = $right);
            assert_eq!(v.operator, $op);
        };
    }
}
