cfg_select! {
    target_family = "unix" => {
        mod unix;
        pub use unix::*;
    }
    target_os = "windows" => {
        mod windows;
        pub use windows::*;
    }
    _ => {
        compile_error!("Unsupported platform. Only Unix-like systems and Windows are currently supported.");
    }
}
