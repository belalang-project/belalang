use std::{
    ffi::CStr,
    os::raw::c_char,
};

use annotate_snippets::{
    AnnotationKind,
    Level,
    Snippet,
    renderer::{
        DecorStyle,
        Renderer,
    },
};

#[repr(C)]
pub struct FFILabel {
    pub span_start: usize,
    pub span_end: usize,
    pub message: *const c_char,
    pub is_primary: bool,
}

#[repr(C)]
pub struct FFIDiagnostic {
    pub severity: u8,
    pub message: *const c_char,
    pub labels: *const FFILabel,
    pub labels_len: usize,
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn belalang_print_diagnostic(
    diag: *const FFIDiagnostic,
    source_text: *const u8,
    source_text_len: usize,
    source_file: *const u8,
    source_file_len: usize,
    use_color: bool,
) {
    let source_text = std::str::from_utf8_unchecked(std::slice::from_raw_parts(source_text, source_text_len));
    let source_file = std::str::from_utf8_unchecked(std::slice::from_raw_parts(source_file, source_file_len));

    let diag = &*diag;
    let message = CStr::from_ptr(diag.message).to_str().unwrap();
    let labels_slice = std::slice::from_raw_parts(diag.labels, diag.labels_len);

    let mut annotations = Vec::new();
    for label in labels_slice {
        let label_msg = CStr::from_ptr(label.message).to_str().unwrap();
        let span = label.span_start..label.span_end;
        let kind = if label.is_primary {
            AnnotationKind::Primary
        } else {
            AnnotationKind::Context
        };
        annotations.push(kind.span(span).label(label_msg));
    }

    let snippet = Snippet::source(source_text)
        .path(source_file)
        .fold(true)
        .annotations(annotations);

    // NOTE: need to match with include/Diag/Diag.h
    let level = match diag.severity {
        0 => Level::ERROR,
        1 => Level::WARNING,
        2 => Level::NOTE,
        3 => Level::HELP,
        _ => Level::ERROR,
    };

    let msg = level.primary_title(message).element(snippet);

    let renderer = if use_color {
        Renderer::styled()
    } else {
        Renderer::plain()
    }
    .decor_style(DecorStyle::Ascii);

    eprintln!("{}", renderer.render(&[msg]));
}
