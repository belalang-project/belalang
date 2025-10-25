//! Belalang Virtual Machine opcodes and instructions.
//!
//! This module defines the bytecode instruction set architecture of The
//! Belalang VM. Each opcode represents an operation that the VM can execute,
//! encoded as single-byte values followed by optional operands.

/// No operation -- Does nothing (1 byte)
pub const NOOP: u8 = 0x00;

/// Stack operation -- Pop from stack (1 byte)
pub const POP: u8 = 0x01;

/// Arithmetic operation -- Add top two stack values (1 byte)
pub const ADD: u8 = 0x10;

/// Arithmetic operation -- Subtract top two stack values (1 byte)
pub const SUB: u8 = 0x11;

/// Arithmetic operation -- Multiply top two stack values (1 byte)
pub const MUL: u8 = 0x12;

/// Arithmetic operation -- Divide top two stack values (1 byte)
pub const DIV: u8 = 0x13;

/// Arithmetic operation -- Modulo of top two stack values (1 byte)
pub const MOD: u8 = 0x14;

/// Constants -- Load constant from constant pool (3 bytes: opcode + 16-bit
/// index)
pub const CONSTANT: u8 = 0x20;

/// Constants -- Push boolean value `true` (1 byte)
pub const TRUE: u8 = 0x21;

/// Constants -- Push boolean value `false` (1 byte)
pub const FALSE: u8 = 0x22;

/// Constants -- Push null value (1 byte)
pub const NULL: u8 = 0x23;

/// Comparison operation -- Compares top two stack values for equality (1 byte)
pub const EQUAL: u8 = 0x30;

/// Comparison operation -- Compares top two stack values for inequality (1
/// byte)
pub const NOT_EQUAL: u8 = 0x31;

/// Comparison operation -- TOS-1 < TOS (1 byte)
pub const LESS_THAN: u8 = 0x32;

/// Comparison operation -- TOS-1 <= TOS (1 byte)
pub const LESS_THAN_EQUAL: u8 = 0x33;

/// Logical operation -- TOS-1 && TOS (1 byte)
pub const AND: u8 = 0x40;

/// Logical operation -- TOS-1 || TOS (1 byte)
pub const OR: u8 = 0x41;

/// Logical operation -- TOS-1 bit and TOS (1 byte)
pub const BIT_AND: u8 = 0x50;

/// Logical operation -- TOS-1 bit or TOS (1 byte)
pub const BIT_OR: u8 = 0x51;

/// Logical operation -- TOS-1 bit xor TOS (1 byte)
pub const BIT_XOR: u8 = 0x52;

/// Logical operation -- TOS-1 << TOS (1 byte)
pub const BIT_SL: u8 = 0x53;

/// Logical operation -- TOS-1 >> TOS (1 byte)
pub const BIT_SR: u8 = 0x54;

/// Unary operation -- !TOS (1 byte)
pub const BANG: u8 = 0x60;

/// Unary operation -- -TOS (1 byte)
pub const MINUS: u8 = 0x61;

/// Jump operation -- Unconditional jump (3 bytes: opcode + 16-bit offset)
pub const JUMP: u8 = 0x70;

/// Jump operation -- Conditional jump if popped TOS is false (3 bytes: opcode +
/// 16-bit offset)
pub const JUMP_IF_FALSE: u8 = 0x71;

/// Print -- Print the TOS value (1 byte)
///
/// See <https://github.com/belalang-project/belalang/issues/38>
pub const PRINT: u8 = 0xD0;

/// Filesystem write -- writes to the filesystem.
pub const FS_WRITE: u8 = 0xD1;

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
        assert_eq!(bytes[0], 32);
        assert_eq!(bytes[1], 255);
        assert_eq!(bytes[2], 254);
    }
}
