#include <wcrn.h>
#include <stdio.h>

int main() {
    // Call the Rust function
    int a = get_stack_size(0, 0);
    printf("stack size(0, 0): %d\n", a);

    return 0;
}