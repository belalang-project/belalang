use crate::SourceSpan;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Severity {
    Error,
    Warning,
    Note,
    Help,
}

#[derive(Debug, Clone)]
pub struct Label {
    pub span: SourceSpan,
    pub message: String,
    pub is_primary: bool,
}

impl Label {
    pub fn primary(span: SourceSpan, message: impl Into<String>) -> Self {
        Self {
            span,
            message: message.into(),
            is_primary: true,
        }
    }

    pub fn secondary(span: SourceSpan, message: impl Into<String>) -> Self {
        Self {
            span,
            message: message.into(),
            is_primary: false,
        }
    }
}

#[derive(Debug, Clone)]
pub struct Diagnostic {
    pub severity: Severity,
    pub message: String,
    pub labels: Vec<Label>,
    pub notes: Vec<String>,
}

impl Diagnostic {
    pub fn error(message: impl Into<String>) -> Self {
        Self {
            severity: Severity::Error,
            message: message.into(),
            labels: Vec::new(),
            notes: Vec::new(),
        }
    }

    pub fn warning(message: impl Into<String>) -> Self {
        Self {
            severity: Severity::Warning,
            message: message.into(),
            labels: Vec::new(),
            notes: Vec::new(),
        }
    }

    pub fn with_label(mut self, label: Label) -> Self {
        self.labels.push(label);
        self
    }

    pub fn with_note(mut self, note: impl Into<String>) -> Self {
        self.notes.push(note.into());
        self
    }
}
