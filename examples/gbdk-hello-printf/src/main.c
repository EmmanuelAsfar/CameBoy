#include <gb/gb.h>
#include <stdio.h>

void main(void) {
    printf("Hello, GBDK!\n");
    while (1) {
        wait_vbl_done();
    }
}


