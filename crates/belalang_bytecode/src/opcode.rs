//! Belalang Virtual Machine opcodes and instructions.
//!
//! This module defines the bytecode instruction set architecture of The
//! Belalang VM. Each opcode represents an operation that the VM can execute,
//! encoded as single-byte values followed by optional operands.

macro_rules! define_opcodes {
    (
        $(#[doc = $doc:expr])*
        $name:ident = $val:expr;
        $($rest:tt)*
    ) => {
        $(#[doc = $doc])*
        pub const $name: u8 = $val;
        define_opcodes!(@step $name; $($rest)*);
    };

    (
        @step $prev:ident;
        $(#[doc = $doc:expr])*
        $name:ident;
        $($rest:tt)*
    ) => {
        $(#[doc = $doc])*
        pub const $name: u8 = $prev + 1;
        define_opcodes!(@step $name; $($rest)*);
    };

    (@step $prev:ident;) => {};
}

define_opcodes! {
    /// No operation -- Does nothing (1 byte)
    NOOP = 0x00;

    /// Stack operation -- Pop from stack (1 byte)
    POP;

    /// Arithmetic operation -- Add top two stack values (1 byte)
    ADD;

    /// Arithmetic operation -- Subtract top two stack values (1 byte)
    SUB;

    /// Arithmetic operation -- Multiply top two stack values (1 byte)
    MUL;

    /// Arithmetic operation -- Divide top two stack values (1 byte)
    DIV;

    /// Arithmetic operation -- Modulo of top two stack values (1 byte)
    MOD;

    /// Constants -- Load constant from constant pool (3 bytes: opcode + 16-bit index)
    CONSTANT;

    /// Constants -- Push boolean value `true` (1 byte)
    TRUE;

    /// Constants -- Push boolean value `false` (1 byte)
    FALSE;

    /// Constants -- Push null value (1 byte)
    NULL;

    /// Comparison operation -- Compares top two stack values for equality (1 byte)
    EQUAL;

    /// Comparison operation -- Compares top two stack values for inequality (1 byte)
    NOT_EQUAL;

    /// Comparison operation -- TOS-1 < TOS (1 byte)
    LESS_THAN;

    /// Comparison operation -- TOS-1 <= TOS (1 byte)
    LESS_THAN_EQUAL;

    /// Logical operation -- TOS-1 && TOS (1 byte)
    AND;

    /// Logical operation -- TOS-1 || TOS (1 byte)
    OR;

    /// Logical operation -- TOS-1 bit and TOS (1 byte)
    BIT_AND;

    /// Logical operation -- TOS-1 bit or TOS (1 byte)
    BIT_OR;

    /// Logical operation -- TOS-1 bit xor TOS (1 byte)
    BIT_XOR;

    /// Logical operation -- TOS-1 << TOS (1 byte)
    BIT_SL;

    /// Logical operation -- TOS-1 >> TOS (1 byte)
    BIT_SR;

    /// Unary operation -- !TOS (1 byte)
    BANG;

    /// Unary operation -- -TOS (1 byte)
    MINUS;

    /// Jump operation -- Unconditional jump (3 bytes: opcode + 16-bit offset)
    JUMP;

    /// Jump operation -- Conditional jump if popped TOS is false (3 bytes: opcode + 16-bit offset)
    JUMP_IF_FALSE;

    /// Print -- Print the TOS value (1 byte)
    ///
    /// See <https://github.com/belalang-project/belalang/issues/38>
    PRINT;

    /// Filesystem write -- writes to the filesystem.
    FS_WRITE;
}

/// Encodes a [`CONSTANT`] instruction with 16-bit index
///
/// # Arguments
/// * `v` - Constant pool index (0-65535)
///
/// # Returns
/// 3-byte array: [[`CONSTANT`], hi_byte, lo_byte]
pub fn constant(v: u16) -> [u8; 3] {
    [CONSTANT, (v >> 8) as u8, (v & 0xFF) as u8]
}

/// Encodes a [`JUMP`] instruction with 16-bit offset
///
/// # Arguments
/// * `v` - The jump offset (0-65535)
///
/// # Returns
/// 3-byte array: [[`JUMP`], hi_byte, lo_byte]
pub fn jump(v: u16) -> [u8; 3] {
    [JUMP, (v >> 8) as u8, (v & 0xFF) as u8]
}

/// Encodes a [`JUMP_IF_FALSE`] instruction with 16-bit offset
///
/// # Arguments
/// * `v` - The jump offset (0-65535)
///
/// # Returns
/// 3-byte array: [[`JUMP_IF_FALSE`], hi_byte, lo_byte]
pub fn jump_if_false(v: u16) -> [u8; 3] {
    [JUMP_IF_FALSE, (v >> 8) as u8, (v & 0xFF) as u8]
}

#[cfg(test)]
mod tests {
    use crate::opcode;

    #[test]
    fn constant() {
        let bytes = opcode::constant(65534);

        assert_eq!(bytes.len(), 3);
        assert_eq!(bytes[0], opcode::CONSTANT);
        assert_eq!(bytes[1], 255);
        assert_eq!(bytes[2], 254);
    }
}
