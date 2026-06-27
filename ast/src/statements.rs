use super::{
    BlockExpression,
    Expression,
};

#[derive(Debug, Clone, Copy)]
pub struct ExpressionStatement<'ast> {
    pub expression: Expression<'ast>,
}

#[derive(Debug, Clone, Copy)]
pub struct ReturnStatement<'ast> {
    pub return_value: Expression<'ast>,
}

#[derive(Debug, Clone, Copy)]
pub struct WhileStatement<'ast> {
    pub condition: &'ast Expression<'ast>,
    pub block: BlockExpression<'ast>,
}

#[derive(Debug, Clone, Copy)]
pub enum Statement<'ast> {
    Expression(ExpressionStatement<'ast>),
    Return(ReturnStatement<'ast>),
    While(WhileStatement<'ast>),
}
