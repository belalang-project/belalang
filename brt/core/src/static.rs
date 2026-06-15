#[unsafe(no_mangle)]
pub extern "C" fn brt_print_int(v: i32) {
    println!("{}", v)
}

#[unsafe(no_mangle)]
pub extern "C" fn brt_print_float(v: f32) {
    println!("{}", v)
}
