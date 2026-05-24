fn main() {
    #[cfg(feature = "mlir")]
    mlir_build();
}

#[cfg(feature = "mlir")]
fn mlir_build() {
    use std::fs;

    use build_rs::{
        input,
        output,
    };

    let manifest_dir = input::cargo_manifest_dir();

    let cmake_build_dir = cmake::Config::new(".")
        .generator("Ninja")
        .define("CMAKE_EXPORT_COMPILE_COMMANDS", "ON")
        .no_build_target(true)
        .build();

    cxx_build::bridge("src/lib.rs")
        .include(manifest_dir.join("include"))
        .compile("belalang-ir-bridge");

    output::rustc_link_search_kind("native", cmake_build_dir.join("build").join("lib"));
    output::rustc_link_lib_kind("static", "BelalangIR");

    // HACK: copy bir-opt to a stable location: target/bir-opt
    let built_bir_opt = cmake_build_dir
        .join("build")
        .join("tools")
        .join("bir-opt")
        .join("bir-opt");
    let target_bir_opt = manifest_dir.join("..").join("..").join("target").join("bir-opt");
    if built_bir_opt.exists() {
        let _ = fs::create_dir_all(target_bir_opt.parent().unwrap());
        let _ = fs::copy(&built_bir_opt, &target_bir_opt);
    }

    output::rerun_if_changed(manifest_dir.join("CMakeLists.txt"));
    output::rerun_if_changed(manifest_dir.join("lib"));
    output::rerun_if_changed(manifest_dir.join("include"));
    output::rerun_if_changed(manifest_dir.join("src"));

    let cmake_json_path = cmake_build_dir.join("build/compile_commands.json");
    let target_json_path = manifest_dir.join("compile_commands.json");
    assert!(fs::copy(&cmake_json_path, &target_json_path).is_ok());
}
