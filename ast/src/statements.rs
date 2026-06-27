use super::{
    BlockExpression,
    Expression,
};

#[derive(Debug, Clone)]
pub struct ExpressionStatement<'ast> {
    pub expression: Expression<'ast>,
}

#[derive(Debug, Clone)]
pub struct ReturnStatement<'ast> {
    pub return_value: Expression<'ast>,
}

#[derive(Debug, Clone)]
pub struct WhileStatement<'ast> {
    pub condition: &'ast Expression<'ast>,
    pub block: BlockExpression<'ast>,
}

#[derive(Debug, Clone)]
pub enum Statement<'ast> {
    Expression(ExpressionStatement<'ast>),
    Return(ReturnStatement<'ast>),
    While(WhileStatement<'ast>),
}
