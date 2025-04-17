use std::mem;

// This file defines the public API for the Rust library.
// It exports functions and types that can be called from C.
// 
use crate::core::stack_tables;
use wacret::core::stack_table::CompiledOp;


#[repr(C)]
pub enum Opcode {
    LocalGet,
    I32Const,
    I64Const,
    F32Const,
    F64Const,
    Call,
    Other,
}

#[repr(C)]
union Operand {
    pub local_idx: u32,
    pub i32_const: i32,
    pub i64_const: i64,
    pub f32_const: u32,
    pub f64_const: u64,
    pub call_result_size: u32,
}

#[repr(C)]
pub struct StackTableEntry {
    pub opcode: Opcode,
    pub ty: u8,
    pub operand: Operand,
}

#[repr(C)]
pub struct StackTable {
    pub size: usize,
    pub entries: *const StackTableEntry,
}

#[repr(C)]
pub struct Array8 {
    pub size: usize,
    pub contents: *const u8,
}

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

#[no_mangle]
pub extern "C" fn wcrn_get_local_types(fidx: u32) -> Array8 {
    let stack_tables = stack_tables::deserialize_stack_table("./")
        .expect("failed to deserialize stack table");
    let locals = stack_tables.get_locals(fidx as usize)
        .expect("failed to get locals")
        .into_iter()
        .map(|ty| ty.size() as u8)
        .collect::<Vec<_>>();
    
    let locals = unsafe {
        Array8 {
            size: locals.len(),
            contents: locals.as_ptr(),
        }
    };
    mem::forget(locals);

    return locals;
}

// TODO: 関数分割する
#[no_mangle]
pub extern "C" fn wcrn_get_stack_table(fidx: u32, offset: u32) -> StackTable {
    let stack_tables = stack_tables::deserialize_stack_table("./")
        .expect("failed to deserialize stack table ({fidx}, {offset})");
    let stack = stack_tables.get_stack(fidx as usize, offset)
        .expect("failed to get stack");
    let size = stack.len();
    let entries = stack.iter()
        .map(|(op, ty)| {
            let (opcode, operand) = match op {
                CompiledOp::LocalGet(idx) => (Opcode::LocalGet, Operand{local_idx: *idx}),
                CompiledOp::I32Const(val) => (Opcode::I32Const, Operand{i32_const: *val}),
                CompiledOp::I64Const(val) => (Opcode::I64Const, Operand{i64_const: *val}),
                CompiledOp::F32Const(val) => (Opcode::F32Const, Operand{f32_const: *val}),
                CompiledOp::F64Const(val) => (Opcode::F64Const, Operand{f64_const: *val}),
                CompiledOp::Call(val) => (Opcode::Call, Operand{call_result_size: *val}),
                _ => (Opcode::Other, Operand{local_idx: 0}), // Handle other cases
            };
            StackTableEntry {
                opcode,
                ty: ty.size(),
                operand,
            }
        })
        .collect::<Vec<_>>();

    let ret = StackTable {
        size,
        entries: entries.as_ptr(),
    };
    mem::forget(entries);
    
    return ret;
    
}