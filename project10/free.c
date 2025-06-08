#include "free.h"
#include "block.h"

void set_free(unsigned char *block, int num, int set) {
    int byte_num = num / 8;
    int bit_num  = num % 8;
    if (set)
        block[byte_num] |=  (1 << bit_num);
    else
        block[byte_num] &= ~(1 << bit_num);
}

int find_free(unsigned char *block) {
    for (int byte = 0; byte < BLOCK_SIZE; byte++) {
        unsigned char b = block[byte];
        if (b != 0xFF) {
            for (int bit = 0; bit < 8; bit++) {
                if (!(b & (1 << bit)))
                    return byte * 8 + bit;
            }
        }
    }
    return -1;
}
