use std::{
    fs::File,
    io::Write,
};

/// Belalalng VM's filesystem model.
#[allow(clippy::upper_case_acronyms)]
pub struct VMFS;

impl Default for VMFS {
    fn default() -> Self {
        Self
    }
}

impl VMFS {
    /// Writes a file based on filename and contents. It will truncate the
    /// contents if the file with name `filename` already exists.
    pub fn write_file(&self, filename: String, contents: String) -> std::io::Result<()> {
        let mut file = File::create(&filename)?;
        file.write_all(contents.as_bytes())?;
        Ok(())
    }
}
