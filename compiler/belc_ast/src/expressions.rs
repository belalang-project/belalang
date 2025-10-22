use belc_lexer::{
    AssignmentKind,
    InfixKind,
    PrefixKind,
};

use super::Statement;

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

impl std::fmt::Display for BooleanExpression {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.value)
    }
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

impl std::fmt::Display for IntegerLiteral {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", &self.value)
    }
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

impl std::fmt::Display for FloatLiteral {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", &self.value)
    }
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
    pub value: String,
}

impl std::fmt::Display for StringLiteral {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.value)
    }
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

impl std::fmt::Display for NullLiteral {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "null")
    }
}

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

impl std::fmt::Display for ArrayLiteral {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let elements = self
            .elements
            .iter()
            .map(|arg| arg.to_string())
            .collect::<Vec<_>>()
            .join(", ");

        write!(f, "[{elements}]")
    }
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

impl std::fmt::Display for VarExpression {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{} {} {};", self.name, self.kind, self.value)
    }
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

impl std::fmt::Display for CallExpression {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let args = self
            .args
            .iter()
            .map(|arg| arg.to_string())
            .collect::<Vec<_>>()
            .join(", ");

        write!(f, "{}({})", self.function, args)
    }
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

impl std::fmt::Display for IndexExpression {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "({}[{}])", self.left, self.index)
    }
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

impl std::fmt::Display for FunctionLiteral {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let params = self
            .params
            .iter()
            .map(|param| param.to_string())
            .collect::<Vec<_>>()
            .join(", ");

        write!(f, "fn({}) {}", params, self.body)
    }
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
    pub value: String,
}

impl std::fmt::Display for Identifier {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", &self.value)
    }
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

impl std::fmt::Display for IfExpression {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "if ({}) {} else {}",
            self.condition,
            self.consequence,
            match &self.alternative {
                Some(alt) => alt.to_string(),
                None => "{}".into(),
            }
        )
    }
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

impl std::fmt::Display for InfixExpression {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "({} {} {})", self.left, self.operator, self.right)
    }
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

impl std::fmt::Display for PrefixExpression {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "({}{})", self.operator, self.right)
    }
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

impl std::fmt::Display for BlockExpression {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let statements = self
            .statements
            .iter()
            .map(|statement| statement.to_string())
            .collect::<Vec<_>>()
            .join(" ");

        write!(f, "{{ {statements} }}")
    }
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
    Call(CallExpression),
    Index(IndexExpression),
    Function(FunctionLiteral),
    Identifier(Identifier),
    If(IfExpression),
    Infix(InfixExpression),
    Prefix(PrefixExpression),
    Block(BlockExpression),
}

impl std::fmt::Display for Expression {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.write_str(&match self {
            Expression::Boolean(v) => v.to_string(),
            Expression::Integer(v) => v.to_string(),
            Expression::Float(v) => v.to_string(),
            Expression::String(v) => v.to_string(),
            Expression::Null(v) => v.to_string(),
            Expression::Array(v) => v.to_string(),
            Expression::Var(v) => v.to_string(),
            Expression::Call(v) => v.to_string(),
            Expression::Index(v) => v.to_string(),
            Expression::Function(v) => v.to_string(),
            Expression::Identifier(v) => v.to_string(),
            Expression::If(v) => v.to_string(),
            Expression::Infix(v) => v.to_string(),
            Expression::Prefix(v) => v.to_string(),
            Expression::Block(v) => v.to_string(),
        })
    }
}
