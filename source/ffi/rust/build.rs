use std::path::Path;

fn main() {
    println!("cargo:rustc-link-search=native=../../../build/display");
    println!("cargo:rustc-link-lib=dylib=display_driver");

    let bindings = bindgen::Builder::default()
        .header("../../display/display_api.h")
        .clang_arg("-I../../display")
        .clang_arg("-I../../lib")
        .generate()
        .expect("Unable to generate bindings");

    let out_path = Path::new("src/bindings.rs");
    bindings
        .write_to_file(out_path)
        .expect("Couldn't write bindings!");
}