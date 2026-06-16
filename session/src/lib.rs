use std::path::PathBuf;

pub struct Session {
    pub source_text: String,
}

impl Session {
    pub fn for_file(input: PathBuf) -> anyhow::Result<Self> {
        let source_text = std::fs::read_to_string(input)?;
        Ok(Self { source_text })
    }

    pub fn for_text(source_text: String) -> anyhow::Result<Self> {
        Ok(Self { source_text })
    }
}
