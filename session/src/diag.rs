use annotate_snippets::{
    AnnotationKind,
    Level,
    Snippet,
    renderer::{
        DecorStyle,
        Renderer,
    },
};

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

pub(crate) fn print_diagnostics(source_text: &str, source_file: &str, diag: &Diagnostic) {
    let mut annotations = Vec::new();
    for label in &diag.labels {
        let span = label.span.start..label.span.end;
        annotations.push(AnnotationKind::Primary.span(span).label(&label.message));
    }

    let snippet = Snippet::source(source_text)
        .path(source_file)
        .fold(true)
        .annotations(annotations);
    let msg = Level::ERROR.primary_title(&diag.message).element(snippet);

    let renderer = get_renderer();
    println!("{}", renderer.render(&[msg]));
}

fn get_renderer() -> Renderer {
    Renderer::plain().decor_style(DecorStyle::Ascii)
}
