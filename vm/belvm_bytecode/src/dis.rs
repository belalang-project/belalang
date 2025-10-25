use crate::opcode;

fn read_u16(bytes: &[u8], i: &mut usize) -> (u16, usize) {
    let hi = bytes[*i + 1];
    let lo = bytes[*i + 2];
    *i += 2;

    ((hi as u16) << 8 | lo as u16, *i - 2)
}

fn read_u8(bytes: &[u8], i: &mut usize) -> (u16, usize) {
    let value = bytes[*i + 1];
    *i += 1;

    (value as u16, *i - 1)
}

pub fn disassemble(bytes: Vec<u8>) -> String {
    let mut result = String::new();
    let mut i = 0;

    while i < bytes.len() {
        match bytes[i] {
            opcode::CONSTANT => {
                let (operand, start_i) = read_u16(&bytes, &mut i);
                result.push_str(&format!("{start_i:#06x}: CONSTANT {operand:#03}\n"));
            },

            opcode::POP => {
                result.push_str(&format!("{i:#06x}: POP\n"));
            },

            opcode::ADD => {
                result.push_str(&format!("{i:#06x}: ADD\n"));
            },

            opcode::SUB => {
                result.push_str(&format!("{i:#06x}: SUB\n"));
            },

            opcode::MUL => {
                result.push_str(&format!("{i:#06x}: MUL\n"));
            },

            opcode::DIV => {
                result.push_str(&format!("{i:#06x}: DIV\n"));
            },

            opcode::MOD => {
                result.push_str(&format!("{i:#06x}: MOD\n"));
            },

            opcode::TRUE => {
                result.push_str(&format!("{i:#06x}: TRUE\n"));
            },

            opcode::FALSE => {
                result.push_str(&format!("{i:#06x}: FALSE\n"));
            },

            opcode::NULL => {
                result.push_str(&format!("{i:#06x}: NULL\n"));
            },

            opcode::EQUAL => {
                result.push_str(&format!("{i:#06x}: EQUAL\n"));
            },

            opcode::NOT_EQUAL => {
                result.push_str(&format!("{i:#06x}: NOT_EQUAL\n"));
            },

            opcode::LESS_THAN => {
                result.push_str(&format!("{i:#06x}: LESS_THAN\n"));
            },

            opcode::LESS_THAN_EQUAL => {
                result.push_str(&format!("{i:#06x}: LESS_THAN_EQUAL\n"));
            },

            opcode::AND => {
                result.push_str(&format!("{i:#06x}: AND\n"));
            },

            opcode::OR => {
                result.push_str(&format!("{i:#06x}: OR\n"));
            },

            opcode::BIT_AND => {
                result.push_str(&format!("{i:#06x}: BIT_AND\n"));
            },

            opcode::BIT_OR => {
                result.push_str(&format!("{i:#06x}: BIT_OR\n"));
            },

            opcode::BIT_XOR => {
                result.push_str(&format!("{i:#06x}: BIT_XOR\n"));
            },

            opcode::BIT_SL => {
                result.push_str(&format!("{i:#06x}: BIT_SL\n"));
            },

            opcode::BIT_SR => {
                result.push_str(&format!("{i:#06x}: BIT_SR\n"));
            },

            opcode::BANG => {
                result.push_str(&format!("{i:#06x}: BANG\n"));
            },

            opcode::MINUS => {
                result.push_str(&format!("{i:#06x}: MINUS\n"));
            },

            opcode::JUMP => {
                let (operand, start_i) = read_u16(&bytes, &mut i);
                result.push_str(&format!("{start_i:#06x}: JUMP {operand:#03}\n"));
            },
            opcode::JUMP_IF_FALSE => {
                let (operand, start_i) = read_u16(&bytes, &mut i);
                result.push_str(&format!("{start_i:#06x}: JUMP_IF_FALSE {operand:#03}\n"));
            },

            opcode::SET_GLOBAL => {
                let (operand, start_i) = read_u16(&bytes, &mut i);
                result.push_str(&format!("{start_i:#06x}: SET_GLOBAL {operand:#03}\n"));
            },

            opcode::GET_GLOBAL => {
                let (operand, start_i) = read_u16(&bytes, &mut i);
                result.push_str(&format!("{start_i:#06x}: GET_GLOBAL {operand:#03}\n"));
            },

            opcode::SET_LOCAL => {
                let (operand, start_i) = read_u8(&bytes, &mut i);
                result.push_str(&format!("{start_i:#06x}: SET_LOCAL {operand:#03}\n"));
            },

            opcode::GET_LOCAL => {
                let (operand, start_i) = read_u8(&bytes, &mut i);
                result.push_str(&format!("{start_i:#06x}: GET_LOCAL {operand:#03}\n"));
            },

            opcode::GET_BUILTIN => {
                let (operand, start_i) = read_u8(&bytes, &mut i);
                result.push_str(&format!("{start_i:#06x}: GET_BUILTIN {operand:#03}\n"));
            },

            opcode::CALL => {
                result.push_str(&format!("{i:#06x}: CALL\n"));
            },

            opcode::RETURN => {
                result.push_str(&format!("{i:#06x}: RETURN\n"));
            },

            opcode::RETURN_VALUE => {
                result.push_str(&format!("{i:#06x}: RETURN_VALUE\n"));
            },

            opcode::MAKE_ARRAY => {
                let (operand, start_i) = read_u16(&bytes, &mut i);
                result.push_str(&format!("{start_i:#06x}: ARRAY {operand:#03}\n"));
            },

            opcode::INDEX => {
                result.push_str(&format!("{i:#06x}: INDEX\n"));
            },

            _ => {},
        }

        i += 1;
    }

    result
}
