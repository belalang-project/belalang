mod buf;
mod vm;

pub use vm::*;

pub trait IntoInstructionBytes {
    fn into_bytes(self) -> Vec<u8>;
}

impl IntoInstructionBytes for u8 {
    fn into_bytes(self) -> Vec<u8> {
        vec![self]
    }
}

impl<const N: usize> IntoInstructionBytes for [u8; N] {
    fn into_bytes(self) -> Vec<u8> {
        self.to_vec()
    }
}

#[macro_export]
macro_rules! instructions {
    ( $( $item:expr ),* $(,)? ) => {{
        let mut instrs = Vec::new();
        $(instrs.extend($item.into_bytes());)*
        instrs
    }};
}
