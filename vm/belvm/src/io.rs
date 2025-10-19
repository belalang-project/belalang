use std::io::Write;

/// Belalalng VM's IO model.
#[allow(clippy::upper_case_acronyms)]
pub struct VMIO;

impl Default for VMIO {
    fn default() -> Self {
        Self
    }
}

impl VMIO {
    /// Prints `msg` to stdout.
    pub fn print(&self, msg: &str) {
        std::io::stdout().write_all(msg.as_bytes()).unwrap();
        std::io::stdout().flush().unwrap();
    }
}
