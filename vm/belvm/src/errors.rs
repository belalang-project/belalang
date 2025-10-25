//! Errors used by The Belalang Virtual Machine.

#[derive(thiserror::Error, Debug)]
pub enum RuntimeError {
    #[error("{0}")]
    Io(#[from] std::io::Error),

    #[error("stack underflow")]
    StackUnderflow,

    #[error("stack overflow")]
    StackOverflow,

    #[error("unknown instruction: {0}")]
    UnknownInstruction(u8),

    #[error("unknown builtin function")]
    UnknownBuiltinFunction,

    // #[error("invalid operation: {0} {1} {2}")]
    // InvalidOperation(Object, String, Object),
    #[error("attempt to call non-function")]
    NotAFunction,

    #[error("Integer overflow")]
    IntegerOverflow,

    #[error("type error")]
    TypeError,

    #[error("allocation failed")]
    AllocationFailed,
}
