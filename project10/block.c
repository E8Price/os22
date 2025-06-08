#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include "image.h"
#include "block.h"
#include "free.h"

static pthread_mutex_t bitmap_lock = PTHREAD_MUTEX_INITIALIZER;

unsigned char *
bread(int block_num, unsigned char *block) {
    off_t offset = (off_t)block_num * BLOCK_SIZE;
    if (lseek(image_fd, offset, SEEK_SET) < 0) return NULL;
    if (read(image_fd, block, BLOCK_SIZE) != BLOCK_SIZE) return NULL;
    return block;
}

void
bwrite(int block_num, unsigned char *block) {
    off_t offset = (off_t)block_num * BLOCK_SIZE;
    lseek(image_fd, offset, SEEK_SET);
    write(image_fd, block, BLOCK_SIZE);
}

int
alloc(void) {
    unsigned char map[BLOCK_SIZE];

    pthread_mutex_lock(&bitmap_lock);

    bread(BLOCK_MAP_BLOCK, map);
    int idx = find_free(map);
    if (idx < 0) {
        pthread_mutex_unlock(&bitmap_lock);
        return -1;
    }
    set_free(map, idx, 1);
    bwrite(BLOCK_MAP_BLOCK, map);

    pthread_mutex_unlock(&bitmap_lock);
    return idx;
}
