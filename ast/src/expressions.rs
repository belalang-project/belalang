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
#[derive(Debug, Clone)]
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
#[derive(Debug, Clone)]
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
#[derive(Debug, Clone)]
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
#[derive(Debug, Clone)]
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
#[derive(Debug, Clone)]
pub struct NullLiteral {}

/// Represents an array literal expression.
///
/// # Examples
///
/// ```belalang
/// [1, 2, 3, "Hello"]
/// ```
#[derive(Debug, Clone)]
pub struct ArrayLiteral {
    pub elements: Vec<Expression>,
}

/// Represents a variable declaration expression.
///
/// # Examples
///
/// ```belalang
/// x := 12
/// y: Int = 12
/// ```
#[derive(Debug, Clone)]
pub struct VarDeclExpression {
    pub name: Identifier,
    pub value: Option<Box<Expression>>,
    pub explicit_ty: Option<Type>,
}

/// Represents an variable assignment literal expression.
///
/// # Examples
///
/// ```belalang
/// x = 12
/// ```
#[derive(Debug, Clone)]
pub struct VarExpression {
    pub kind: AssignmentKind,
    pub name: Identifier,
    pub value: Box<Expression>,
}

/// Represents a function call expression.
///
/// # Examples
///
/// ```belalang
/// foo()
/// ```
#[derive(Debug, Clone)]
pub struct CallExpression {
    pub function: Box<Expression>,
    pub args: Vec<Expression>,
}

/// Represents an indexing expression.
///
/// # Examples
///
/// ```belalang
/// foo[1]
/// ```
#[derive(Debug, Clone)]
pub struct IndexExpression {
    pub left: Box<Expression>,
    pub index: Box<Expression>,
}

/// Represents a function expression.
///
/// # Examples
///
/// ```belalang
/// fn() {}
/// ```
#[derive(Debug, Clone)]
pub struct FunctionLiteral {
    pub params: Vec<Identifier>,
    pub body: BlockExpression,
}

/// Represents an identifier expression.
///
/// # Examples
///
/// ```belalang
/// foo
/// ```
#[derive(Debug, Clone)]
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
#[derive(Debug, Clone)]
pub struct IfExpression {
    pub condition: Box<Expression>,
    pub consequence: BlockExpression,
    pub alternative: Option<Box<Expression>>,
}

/// Represents an infix expression.
///
/// # Examples
///
/// ```belalang
/// 1 + 1
/// ```
#[derive(Debug, Clone)]
pub struct InfixExpression {
    pub left: Box<Expression>,
    pub operator: InfixKind,
    pub right: Box<Expression>,
}

/// Represents an prefix expression.
///
/// # Examples
///
/// ```belalang
/// -1
/// ```
#[derive(Debug, Clone)]
pub struct PrefixExpression {
    pub operator: PrefixKind,
    pub right: Box<Expression>,
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
#[derive(Debug, Clone)]
pub struct BlockExpression {
    pub statements: Vec<Statement>,
}

/// Represents all expressions supported by The Belalang Compiler.
#[derive(Debug, Clone)]
pub enum Expression {
    Boolean(BooleanExpression),
    Integer(IntegerLiteral),
    Float(FloatLiteral),
    String(StringLiteral),
    Null(NullLiteral),
    Array(ArrayLiteral),
    Var(VarExpression),
    VarDecl(VarDeclExpression),
    Call(CallExpression),
    Index(IndexExpression),
    Function(FunctionLiteral),
    Identifier(Identifier),
    If(IfExpression),
    Infix(InfixExpression),
    Prefix(PrefixExpression),
    Block(BlockExpression),
}
