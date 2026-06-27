use super::{
    BlockExpression,
    Expression,
};

#[derive(Debug, Clone)]
pub struct ExpressionStatement {
    pub expression: Expression,
}

#[derive(Debug, Clone)]
pub struct ReturnStatement {
    pub return_value: Expression,
}

#[derive(Debug, Clone)]
pub struct WhileStatement {
    pub condition: Box<Expression>,
    pub block: BlockExpression,
}

#[derive(Debug, Clone)]
pub enum Statement {
    Expression(ExpressionStatement),
    Return(ReturnStatement),
    While(WhileStatement),
}
