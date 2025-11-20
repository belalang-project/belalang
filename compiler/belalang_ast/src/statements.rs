use super::{
    BlockExpression,
    Expression,
};

#[derive(Debug, Clone)]
pub struct ExpressionStatement {
    pub expression: Expression,
}

impl std::fmt::Display for ExpressionStatement {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{};", &self.expression)
    }
}

#[derive(Debug, Clone)]
pub struct ReturnStatement {
    pub return_value: Expression,
}

impl std::fmt::Display for ReturnStatement {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "return {};", self.return_value)
    }
}

#[derive(Debug, Clone)]
pub struct WhileStatement {
    pub condition: Box<Expression>,
    pub block: BlockExpression,
}

impl std::fmt::Display for WhileStatement {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "while ({}) {}", self.condition, self.block)
    }
}

#[derive(Debug, Clone)]
pub enum Statement {
    Expression(ExpressionStatement),
    Return(ReturnStatement),
    While(WhileStatement),
}

impl std::fmt::Display for Statement {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let value = match self {
            Statement::Expression(v) => v.to_string(),
            Statement::Return(v) => v.to_string(),
            Statement::While(v) => v.to_string(),
        };

        f.write_str(&value)
    }
}
