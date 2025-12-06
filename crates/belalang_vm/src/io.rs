use std::io::{
    self,
    Write,
};

#[allow(clippy::upper_case_acronyms)]
pub struct VMIO {
    out_stream: Box<dyn Write + Send>,
}

impl Default for VMIO {
    fn default() -> Self {
        Self::new(Box::new(io::stdout()))
    }
}

impl VMIO {
    pub fn new(out_stream: Box<dyn Write + Send>) -> Self {
        Self { out_stream }
    }

    pub fn print(&mut self, msg: &str) {
        self.out_stream.write_all(msg.as_bytes()).unwrap();
        self.out_stream.flush().unwrap();
    }
}
