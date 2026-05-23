#[cfg(feature = "mlir")]
#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {}
}

#[cfg(not(feature = "mlir"))]
pub mod ffi {}
