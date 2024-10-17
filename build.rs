fn main() {
    cc::Build::new()
        .file("src/multifit_nlinear.c")
        .include("./gslsyslib/target/debug")
        .flag("-lgslsyslib")
        .compile("gslmfnlin");
    println!("cargo::rustc-link-search=./target/debug");
    println!("cargo::rustc-link-lib=gslmfnlin");
    println!("cargo::rerun-if-changed=src/multifit_nlinear.c");
}
