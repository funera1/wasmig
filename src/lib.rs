#[no_mangle]
pub extern "C" fn add(left: u64, right: u64) -> u64 {
    left + right
}

#[no_mangle]
pub extern "C" fn helloworld() {
    println!("Hello World");
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
