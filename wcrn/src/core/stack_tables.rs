extern crate wacret;

use wacret::core::{function_v2::{CodePos, Function, Stack}, stack_table::StackTables};
use anyhow::Result;
use std::{fs::File, io::Read};

pub fn hello() {
    println!("Hello from the core module!");    
}

pub fn deserialize_stack_table<P>(path: P) -> Result<StackTables> 
where 
    P: AsRef<std::path::Path>,
{
    // stack-table.msgpackをread
    let full_path = path.as_ref().join("stack-table.msgpack");
    let mut file = File::open(full_path)?;
    let mut data = Vec::new();
    file.read_to_end(&mut data)?;
    
    // dataをもとにStackTablesを生成
    let stack_table = StackTables::deserialize(&data);
    Ok(stack_table)
}

// stack-tablesからあるコード位置のstack sizeを取得する
pub fn get_stack_size<P>(path: P, fidx: u32, offset: u32) -> Result<usize> 
where 
    P: AsRef<std::path::Path>,
{
    let stack_tables = deserialize_stack_table(path)?;
    let stack = stack_tables.get_stack(fidx as usize, offset)?;
    Ok(stack.len())
}