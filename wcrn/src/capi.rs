// This file defines the public API for the Rust library.
// It exports functions and types that can be called from C.
// 
use crate::core::stack_tables;

#[no_mangle]
pub extern "C" fn wcrn_rust_function() -> i32 {
    // Example function that returns an integer
    stack_tables::hello();
    42
}

#[no_mangle]
pub extern "C" fn wcrn_get_stack_size(fidx: u32, offset: u32) -> usize {
    stack_tables::get_stack_size("./", fidx, offset)
        .expect("failed to get stack size ({fidx}, {offset})")
}