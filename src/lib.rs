use std::fs::File;
use std::io::{self, Write, Seek, SeekFrom};
use std::path::Path;
use rustix::procfs::proc_self_maps;

const WASM_PAGE_SIZE: u32 = 0x10000;

#[no_mangle]
pub extern "C" fn add(left: u64, right: u64) -> u64 {
    left + right
}

#[no_mangle]
pub extern "C" fn helloworld() {
    println!("Hello World");
}

fn write_dirty_memory(memory: &[u8], mem_addr: u32) {
    const PAGEMAP_LENGTH: u32 = 8;
    const OS_PAGE_SIZE: u32 = 4096;

    let path = Path::new("memory.img");
    let mut f = File::create(path).expect("Failed to create file");

    let mut pagemap = File::open("/proc/self/pagemap").expect("Failed to open pagemap");
    let entry_offset =  (mem_addr / OS_PAGE_SIZE) * PAGEMAP_LENGTH;
    pagemap.seek(SeekFrom::Start(entry_offset as u64));

}

#[no_mangle]
pub extern "C" fn checkpoint_memory(memory: *const u8, cur_page: u32) -> i32 {
    // ポインタがNULLかどうかを確認
    if memory.is_null() {
        return -1; // エラーコード: 無効なポインタ
    }

    // ポインタをスライスに変換（unsafeブロックが必要）
    let mem_addr = memory as usize;
    let memory = unsafe { std::slice::from_raw_parts(memory, (cur_page * WASM_PAGE_SIZE) as usize) };
}

#[no_mangle]
pub extern "C" fn checkpoint_global(values: *const u64, types: *const u32, len: usize) -> i32 {
    // ポインタがNULLかどうかを確認
    if values.is_null() || types.is_null() {
        return -1; // エラーコード: 無効なポインタ
    }

    // ポインタをスライスに変換（unsafeブロックが必要）
    let values = unsafe { std::slice::from_raw_parts(values, len) };
    let types = unsafe { std::slice::from_raw_parts(types, len) };

    // ファイル操作
    match File::create("globals.img").and_then(|mut file| {
        for (&value, &type_) in values.iter().zip(types.iter()) {
            file.write_all(&value.to_le_bytes())?;
            file.write_all(&type_.to_le_bytes())?;
        }
        Ok(())
    }) {
        Ok(_) => 0,  // 成功: 0を返す
        Err(_) => 1, // エラーコード: 書き込み失敗
    }
}

#[no_mangle]
pub extern "C" fn checkpoint_pc(fidx: u32, offset: u32) -> i32 {
    let path = Path::new("program_counter.img");
    let mut f = File::create(path).expect("Failed to create file");

    f.write_all(&fidx.to_le_bytes()).expect("Failed to write fidx");
    f.write_all(&offset.to_le_bytes()).expect("Failed to write offset");

    return 0;
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn it_works() {
        let result = add(2, 2);
        assert_eq!(result, 4);
    }
}
