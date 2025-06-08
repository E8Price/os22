#include <string.h>
#include <pthread.h>
#include "inode.h"
#include "block.h"
#include "free.h"
#include "pack.h"

static pthread_mutex_t inodemap_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t incore_lock  = PTHREAD_MUTEX_INITIALIZER;

static struct inode incore[MAX_SYS_OPEN_FILES];

static void
inode_loc(unsigned int inode_num,
          int *block_num, int *byte_offset) {
    unsigned int idx = inode_num / INODES_PER_BLOCK;
    *block_num      = idx + INODE_FIRST_BLOCK;
    unsigned int off = inode_num % INODES_PER_BLOCK;
    *byte_offset    = off * INODE_SIZE;
}

struct inode *
incore_find_free(void) {
    for (int i = 0; i < MAX_SYS_OPEN_FILES; i++)
        if (incore[i].ref_count == 0)
            return &incore[i];
    return NULL;
}

struct inode *
incore_find(unsigned int inode_num) {
    for (int i = 0; i < MAX_SYS_OPEN_FILES; i++)
        if (incore[i].ref_count > 0 &&
            incore[i].inode_num == inode_num)
            return &incore[i];
    return NULL;
}

void
incore_free_all(void) {
    for (int i = 0; i < MAX_SYS_OPEN_FILES; i++)
        incore[i].ref_count = 0;
}

void
read_inode(struct inode *in, unsigned int inode_num) {
    unsigned char block[BLOCK_SIZE];
    int bnum, off;
    inode_loc(inode_num, &bnum, &off);
    bread(bnum, block);
    in->size        = read_u32(block + off + 0);
    in->owner_id    = read_u16(block + off + 4);
    in->permissions = read_u8(block + off + 6);
    in->flags       = read_u8(block + off + 7);
    in->link_count  = read_u8(block + off + 8);
    for (int i = 0; i < INODE_PTR_COUNT; i++) {
        int ptr = 9 + i * 2;
        in->block_ptr[i] = read_u16(block + off + ptr);
    }
}

void
write_inode(const struct inode *in) {
    unsigned char block[BLOCK_SIZE];
    int bnum, off;
    inode_loc(in->inode_num, &bnum, &off);
    bread(bnum, block);
    write_u32(block + off + 0, in->size);
    write_u16(block + off + 4, in->owner_id);
    write_u8 (block + off + 6, in->permissions);
    write_u8 (block + off + 7, in->flags);
    write_u8 (block + off + 8, in->link_count);
    for (int i = 0; i < INODE_PTR_COUNT; i++) {
        int ptr = 9 + i * 2;
        write_u16(block + off + ptr, in->block_ptr[i]);
    }
    bwrite(bnum, block);
}

struct inode *
iget(unsigned int inode_num) {
    pthread_mutex_lock(&incore_lock);

    struct inode *in = incore_find(inode_num);
    if (in) {
        in->ref_count++;
        pthread_mutex_unlock(&incore_lock);
        return in;
    }

    in = incore_find_free();
    if (!in) {
        pthread_mutex_unlock(&incore_lock);
        return NULL;
    }
    in->ref_count  = 1;
    in->inode_num = inode_num;
    pthread_mutex_unlock(&incore_lock);

    read_inode(in, inode_num);
    return in;
}

void
iput(struct inode *in) {
    if (!in) return;

    pthread_mutex_lock(&incore_lock);
    if (in->ref_count > 0) {
        in->ref_count--;
        if (in->ref_count == 0) {
            pthread_mutex_unlock(&incore_lock);
            write_inode(in);
            return;
        }
    }
    pthread_mutex_unlock(&incore_lock);
}

struct inode *
ialloc(void) {
    unsigned char map[BLOCK_SIZE];

    pthread_mutex_lock(&inodemap_lock);
    bread(INODE_MAP_BLOCK, map);
    int idx = find_free(map);
    if (idx < 0) {
        pthread_mutex_unlock(&inodemap_lock);
        return NULL;
    }
    set_free(map, idx, 1);
    bwrite(INODE_MAP_BLOCK, map);
    pthread_mutex_unlock(&inodemap_lock);

    struct inode *in = iget(idx);
    if (!in) return NULL;

    in->size        = 0;
    in->owner_id    = 0;
    in->permissions = 0;
    in->flags       = 0;
    in->link_count  = 0;
    for (int i = 0; i < INODE_PTR_COUNT; i++)
        in->block_ptr[i] = 0;

    write_inode(in);
    return in;
}
