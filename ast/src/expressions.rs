use lexer::{
    AssignmentKind,
    InfixKind,
    PrefixKind,
};
use session::interner::Symbol;

use super::Statement;
use crate::type_inferer::Type;

/// Represents a boolean literal expression.
///
/// # Examples
///
/// ```belalang
/// true
/// ```
///
/// ```belalang
/// false
/// ```
#[derive(Debug, Clone, Copy)]
pub struct BooleanExpression {
    pub value: bool,
}

/// Represents an integer literal expression.
///
/// # Examples
///
/// ```belalang
/// 42
/// ```
#[derive(Debug, Clone, Copy)]
pub struct IntegerLiteral {
    pub value: i64,
}

/// Represents an float literal expression.
///
/// # Examples
///
/// ```belalang
/// 3.14
/// ```
#[derive(Debug, Clone, Copy)]
pub struct FloatLiteral {
    pub value: f64,
}

/// Represents an string literal expression.
///
/// # Examples
///
/// ```belalang
/// "hello, world"
/// ```
#[derive(Debug, Clone, Copy)]
pub struct StringLiteral {
    pub value: Symbol,
}

/// Represents an null literal expression.
///
/// # Examples
///
/// ```belalang
/// null
/// ```
#[derive(Debug, Clone, Copy)]
pub struct NullLiteral {}

/// Represents an array literal expression.
///
/// # Examples
///
/// ```belalang
/// [1, 2, 3, "Hello"]
/// ```
#[derive(Debug, Clone, Copy)]
pub struct ArrayLiteral<'ast> {
    pub elements: &'ast [Expression<'ast>],
}

/// Represents an variable assignment literal expression.
///
/// # Examples
///
/// ```belalang
/// x = 12
/// ```
#[derive(Debug, Clone, Copy)]
pub struct VarExpression<'ast> {
    pub kind: AssignmentKind,
    pub name: Identifier,
    pub value: &'ast Expression<'ast>,
}

/// Represents a function call expression.
///
/// # Examples
///
/// ```belalang
/// foo()
/// ```
#[derive(Debug, Clone, Copy)]
pub struct CallExpression<'ast> {
    pub function: &'ast Expression<'ast>,
    pub args: &'ast [Expression<'ast>],
}

/// Represents an indexing expression.
///
/// # Examples
///
/// ```belalang
/// foo[1]
/// ```
#[derive(Debug, Clone, Copy)]
pub struct IndexExpression<'ast> {
    pub left: &'ast Expression<'ast>,
    pub index: &'ast Expression<'ast>,
}

/// Represents a function expression.
///
/// # Examples
///
/// ```belalang
/// fn() {}
/// ```
#[derive(Debug, Clone, Copy)]
pub struct FunctionLiteral<'ast> {
    pub params: &'ast [Identifier],
    pub body: BlockExpression<'ast>,
}

/// Represents an identifier expression.
///
/// # Examples
///
/// ```belalang
/// foo
/// ```
#[derive(Debug, Clone, Copy)]
pub struct Identifier {
    pub value: Symbol,
}

/// Represents an if expression.
///
/// # Examples
///
/// ```belalang
/// if () {} else {}
/// ```
#[derive(Debug, Clone, Copy)]
pub struct IfExpression<'ast> {
    pub condition: &'ast Expression<'ast>,
    pub consequence: BlockExpression<'ast>,
    pub alternative: Option<&'ast Expression<'ast>>,
}

/// Represents an infix expression.
///
/// # Examples
///
/// ```belalang
/// 1 + 1
/// ```
#[derive(Debug, Clone, Copy)]
pub struct InfixExpression<'ast> {
    pub left: &'ast Expression<'ast>,
    pub operator: InfixKind,
    pub right: &'ast Expression<'ast>,
}

/// Represents an prefix expression.
///
/// # Examples
///
/// ```belalang
/// -1
/// ```
#[derive(Debug, Clone, Copy)]
pub struct PrefixExpression<'ast> {
    pub operator: PrefixKind,
    pub right: &'ast Expression<'ast>,
}

/// Represents an code block expression.
///
/// This is used in while statements, if expressions, and etc that needs a code
/// block.
///
/// # Examples
///
/// ```belalang
/// {}
/// ```
#[derive(Debug, Clone, Copy)]
pub struct BlockExpression<'ast> {
    pub statements: &'ast [Statement<'ast>],
}

/// Represents all expressions supported by The Belalang Compiler.
#[derive(Debug, Clone, Copy)]
pub enum Expression<'ast> {
    Boolean(BooleanExpression),
    Integer(IntegerLiteral),
    Float(FloatLiteral),
    String(StringLiteral),
    Null(NullLiteral),
    Array(ArrayLiteral<'ast>),
    Var(VarExpression<'ast>),
    Call(CallExpression<'ast>),
    Index(IndexExpression<'ast>),
    Function(FunctionLiteral<'ast>),
    Identifier(Identifier),
    If(IfExpression<'ast>),
    Infix(InfixExpression<'ast>),
    Prefix(PrefixExpression<'ast>),
    Block(BlockExpression<'ast>),
}
