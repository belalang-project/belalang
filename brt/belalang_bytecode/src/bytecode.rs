//! Bytecode used by The Belalang VM.
//!
//! Defines the structure and components used to represent compiled bytecode,
//! including instructions and constants.

use std::io::{
    Cursor,
    Read,
};

use bincode::{
    Decode,
    Encode,
    config,
    decode_from_slice,
    encode_to_vec,
};
use crc32fast::Hasher;

static BEL_MAGIC: [u8; 4] = [0xBEu8, 0x1Au8, 0x1Au8, 0x9Cu8];
static BEL_VERSION: u16 = 1;

/// Constants used in the Belalang bytecode
///
/// These values are used to represent literal data embedded in the bytecode.
/// They are referenced by index in the constant pool during execution.
#[derive(Encode, Decode, Debug, Default, Clone, PartialEq)]
pub enum Constant {
    #[default]
    Null,
    Integer(i64),
    Boolean(bool),
    String(String),
}

/// A compiled bytecode object for the Belalang VM
///
/// This contains the instruction stream and associated constant pool needed for
/// execution by the virtual machine.
#[derive(Encode, Decode)]
pub struct Bytecode {
    /// The instructions to be executed
    ///
    /// Each byte corresponds to an opcode defined in [crate::core::opcode].
    pub instructions: Vec<u8>,

    /// Constant values referenced by the bytecode
    pub constants: Vec<Constant>,
}

/// List of errors when decoding bytes into [`Bytecode`].
#[derive(thiserror::Error, Debug)]
pub enum BytecodeDecodeError {
    #[error("invalid checksum")]
    Checksum,
    #[error("invalid magic number")]
    MagicNumber,
    #[error("invalid version")]
    Version,
}

impl Bytecode {
    /// Encodes bytecode into an array of bytes.
    pub fn into_bytes(self) -> Vec<u8> {
        let mut buffer = Vec::new();

        let config = config::standard();
        let encoded = encode_to_vec(self, config).unwrap();

        let mut hasher = Hasher::new();
        hasher.update(&encoded);
        let checksum = hasher.finalize();

        buffer.extend_from_slice(&BEL_MAGIC);
        buffer.extend_from_slice(&BEL_VERSION.to_le_bytes());
        buffer.extend(checksum.to_le_bytes());
        buffer.extend(encoded);

        buffer
    }

    /// Decodes bytecode into an array of bytes.
    pub fn from_bytes(buffer: &[u8]) -> Result<Self, BytecodeDecodeError> {
        let mut cursor = Cursor::new(buffer);

        let mut magic = [0u8; 4];
        cursor.read_exact(&mut magic).unwrap();
        if magic != BEL_MAGIC {
            return Err(BytecodeDecodeError::MagicNumber);
        }

        let mut version = [0u8; 2];
        cursor.read_exact(&mut version).unwrap();
        let version = u16::from_le_bytes(version);
        if version != BEL_VERSION {
            return Err(BytecodeDecodeError::Version);
        }

        let mut stored_cksum = [0u8; 4];
        cursor.read_exact(&mut stored_cksum).unwrap();
        let stored_cksum = u32::from_le_bytes(stored_cksum);

        let start = cursor.position() as usize;
        let encoded_data = &buffer[start..];
        let mut hasher = Hasher::new();
        hasher.update(encoded_data);
        let computed_cksum = hasher.finalize();

        if stored_cksum != computed_cksum {
            return Err(BytecodeDecodeError::Checksum);
        }

        let config = config::standard();
        let (decoded, _) = decode_from_slice(encoded_data, config).unwrap();

        Ok(decoded)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn bytecode_encode_roundtrip() {
        let instructions = vec![1, 2, 3, 4, 5];
        let constants = vec![
            Constant::Null,
            Constant::Integer(12345),
            Constant::Boolean(true),
            Constant::String("hello world".to_string()),
        ];

        let original_bytecode = Bytecode {
            instructions: instructions.clone(),
            constants: constants.clone(),
        };

        let bytes = original_bytecode.into_bytes();
        let decoded_bytecode = Bytecode::from_bytes(&bytes).unwrap();

        assert_eq!(decoded_bytecode.instructions, instructions);
        assert_eq!(decoded_bytecode.constants, constants);
    }

    #[test]
    fn errors_invalid_magic_number() {
        let instructions = vec![1, 2, 3, 4, 5];
        let constants = vec![
            Constant::Null,
            Constant::Integer(12345),
            Constant::Boolean(true),
            Constant::String("hello world".to_string()),
        ];

        let original_bytecode = Bytecode {
            instructions: instructions.clone(),
            constants: constants.clone(),
        };

        let mut bytes = original_bytecode.into_bytes();
        bytes[0] = 0x00;

        assert!(matches!(
            Bytecode::from_bytes(&bytes),
            Err(BytecodeDecodeError::MagicNumber)
        ));
    }

    #[test]
    fn errors_invalid_version() {
        let instructions = vec![1, 2, 3, 4, 5];
        let constants = vec![
            Constant::Null,
            Constant::Integer(12345),
            Constant::Boolean(true),
            Constant::String("hello world".to_string()),
        ];

        let original_bytecode = Bytecode {
            instructions: instructions.clone(),
            constants: constants.clone(),
        };

        let mut bytes = original_bytecode.into_bytes();
        bytes[5] = 0xFF;

        assert!(matches!(
            Bytecode::from_bytes(&bytes),
            Err(BytecodeDecodeError::Version)
        ));
    }

    #[test]
    fn errors_invalid_cksum() {
        let instructions = vec![1, 2, 3, 4, 5];
        let constants = vec![
            Constant::Null,
            Constant::Integer(12345),
            Constant::Boolean(true),
            Constant::String("hello world".to_string()),
        ];

        let original_bytecode = Bytecode {
            instructions: instructions.clone(),
            constants: constants.clone(),
        };

        let mut bytes = original_bytecode.into_bytes();
        bytes[6] = 0x00;
        bytes[7] = 0x00;
        bytes[8] = 0x00;
        bytes[9] = 0x00;

        assert!(matches!(
            Bytecode::from_bytes(&bytes),
            Err(BytecodeDecodeError::Checksum)
        ));
    }
}
