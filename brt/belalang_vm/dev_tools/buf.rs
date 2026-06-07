use std::{
    io,
    sync::{
        Arc,
        Mutex,
    },
};

#[derive(Clone, Default)]
pub struct SharedBuffer {
    buffer: Arc<Mutex<Vec<u8>>>,
}

impl SharedBuffer {
    pub fn get_string(&self) -> String {
        let guard = self.buffer.lock().unwrap();
        String::from_utf8(guard.clone()).unwrap_or_else(|_| "<Invalid UTF-8>".to_string())
    }

    #[allow(dead_code)]
    pub fn clear(&self) {
        let mut guard = self.buffer.lock().unwrap();
        guard.clear();
    }
}

impl io::Write for SharedBuffer {
    fn write(&mut self, buf: &[u8]) -> io::Result<usize> {
        let mut guard = self.buffer.lock().unwrap();
        guard.extend_from_slice(buf);
        Ok(buf.len())
    }

    fn flush(&mut self) -> io::Result<()> {
        Ok(())
    }
}
