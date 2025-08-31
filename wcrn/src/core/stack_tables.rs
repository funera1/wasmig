extern crate wacret;

use wacret::core::stack_table::StackTables;
use anyhow::{Result, anyhow};
use std::{fs::File, io::Read, path::Path};
use once_cell::sync::OnceCell;

// Global cache for a single StackTables instance loaded once and reused.
static STACK_TABLES: OnceCell<StackTables> = OnceCell::new();

pub fn hello() {
    println!("Hello from the core module!");    
}

fn load_from_path(path: &Path) -> Result<StackTables> {
    let full_path = path.join("stack-table.msgpack");
    let mut file = File::open(&full_path)?;
    let mut data = Vec::new();
    file.read_to_end(&mut data)?;
    Ok(StackTables::deserialize(&data))
}

/// 初回のみstack-table.msgpackを読み込みキャッシュする。
pub fn init_stack_tables<P: AsRef<Path>>(path: P) -> Result<()> {
    let path_ref = path.as_ref();
    STACK_TABLES
        .get_or_try_init(|| load_from_path(path_ref))
        .map(|_| ())
}

/// 既に初期化済みのStackTablesへの参照を返す。未初期化ならデフォルトパス("./")で読み込む。
pub fn get_stack_tables() -> Result<&'static StackTables> {
    if let Some(t) = STACK_TABLES.get() { return Ok(t); }
    // デフォルトパス
    init_stack_tables("./")?;
    STACK_TABLES.get().ok_or_else(|| anyhow!("StackTables not initialized"))
}

/// (互換用) 常にパスを受け取るが、実際には一度だけロードして以降はキャッシュを使う。
pub fn deserialize_stack_table<P>(path: P) -> Result<&'static StackTables>
where
    P: AsRef<Path>,
{
    init_stack_tables(path)?;
    get_stack_tables()
}

// stack-tablesからあるコード位置のstack sizeを取得する
pub fn get_stack_size<P>(path: P, fidx: u32, offset: u32) -> Result<usize>
where
    P: AsRef<Path>,
{
    let stack_tables = deserialize_stack_table(path)?; // this now returns cached reference
    let stack = stack_tables.get_stack(fidx as usize, offset)?;
    Ok(stack.len())
}

/// キャッシュ済みStackTablesからstack sizeを取得する(初期化は呼び出し側で済ませている前提)。
pub fn get_stack_size_cached(fidx: u32, offset: u32) -> Result<usize> {
    let stack_tables = get_stack_tables()?;
    let stack = stack_tables.get_stack(fidx as usize, offset)?;
    Ok(stack.len())
}