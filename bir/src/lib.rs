pub use ffi::*;

#[cxx::bridge(namespace = "bir")]
mod ffi {
    unsafe extern "C++" {
        include!("belalang/IR/Builder.h");

        type BIRValue;
        type BIRBuilder;

        fn create_builder() -> UniquePtr<BIRBuilder>;

        fn build_constant_int(self: Pin<&mut BIRBuilder>, val: i64) -> UniquePtr<BIRValue>;
        fn build_constant_float(self: Pin<&mut BIRBuilder>, val: f64) -> UniquePtr<BIRValue>;
        fn build_add(self: Pin<&mut BIRBuilder>, lhs: &BIRValue, rhs: &BIRValue) -> UniquePtr<BIRValue>;
        fn build_sub(self: Pin<&mut BIRBuilder>, lhs: &BIRValue, rhs: &BIRValue) -> UniquePtr<BIRValue>;
        fn build_mul(self: Pin<&mut BIRBuilder>, lhs: &BIRValue, rhs: &BIRValue) -> UniquePtr<BIRValue>;
        fn build_div(self: Pin<&mut BIRBuilder>, lhs: &BIRValue, rhs: &BIRValue) -> UniquePtr<BIRValue>;
        fn build_mod(self: Pin<&mut BIRBuilder>, lhs: &BIRValue, rhs: &BIRValue) -> UniquePtr<BIRValue>;
        fn build_print(self: Pin<&mut BIRBuilder>, val: &BIRValue);

        fn dump(self: &BIRBuilder);
        fn dump_to_string(self: &BIRBuilder) -> String;
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_builder() {
        let mut builder = create_builder();
        let lhs = builder.pin_mut().build_constant_int(42);
        let rhs = builder.pin_mut().build_constant_int(1);
        let sum = builder.pin_mut().build_add(&lhs, &rhs);
        builder.pin_mut().build_print(&sum);

        let ir = builder.dump_to_string();
        assert!(ir.contains("bir.constant 42"));
        assert!(ir.contains("bir.constant 1"));
        assert!(ir.contains("bir.add"));
        assert!(ir.contains("bir.print"));
    }
}
