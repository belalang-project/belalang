#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
        include!("belalang_ir/test.h");

        fn hello() -> u64;
    }
}

pub fn hello_from_rust() -> u64 {
    ffi::hello()
}

#[cfg(test)]
mod tests {
    #[test]
    fn correct() {
        assert_eq!(super::hello_from_rust(), 1);
    }
}
