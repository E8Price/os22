#include "ctest.h"

#include "image.h"
#include "block.h"
#include "free.h"
#include "inode.h"

CTEST(test_free, find_and_set) {
    unsigned char m[BLOCK_SIZE] = {0};
    CTEST_ASSERT(find_free(m) == 0, "first free is 0");
    set_free(m, 0, 1);
    CTEST_ASSERT(find_free(m) == 1, "next free is 1");
}

CTEST(inode_incore, find_and_free) {
    incore_free_all();
    struct inode *f = incore_find_free();
    CTEST_ASSERT(f != NULL, "got a free slot");
    f->ref_count = 1;
    struct inode *g = incore_find_free();
    CTEST_ASSERT(g != f, "next free is different");
}

CTEST(inode_readwrite, round_trip) {
    CTEST_ASSERT(image_open("img", 1) >= 0, "open image");
    struct inode in = {
        .size        = 1234,
        .owner_id    = 42,
        .permissions = 7,
        .flags       = 1,
        .link_count  = 2,
        .inode_num   = 5
    };
    for (int i = 0; i < INODE_PTR_COUNT; i++) {
        in.block_ptr[i] = i;
    }
    write_inode(&in);

    incore_free_all();
    struct inode *out = iget(5);
    CTEST_ASSERT(out != NULL, "iget succeeded");
    CTEST_ASSERT(out->size == 1234, "size matches");
    CTEST_ASSERT(out->owner_id == 42, "owner matches");
    CTEST_ASSERT(out->block_ptr[7] == 7, "block ptr ok");
    CTEST_ASSERT(image_close() >= 0, "close image");
}

CTEST(inode_alloc, simple_ialloc) {
    unsigned char m[BLOCK_SIZE] = {0};
    CTEST_ASSERT(image_open("img", 1) >= 0, "open image");
    bwrite(INODE_MAP_BLOCK, m);
    struct inode *n1 = ialloc();
    CTEST_ASSERT(n1 != NULL, "ialloc returned inode");
    CTEST_ASSERT(n1->inode_num == 0, "first inode num is 0");
    struct inode *n2 = ialloc();
    CTEST_ASSERT(n2 != NULL, "ialloc returned second inode");
    CTEST_ASSERT(n2->inode_num == 1, "second inode num is 1");
    CTEST_ASSERT(image_close() >= 0, "close image");
}

CTEST(inode_iput, write_on_zero) {
    CTEST_ASSERT(image_open("img", 1) >= 0, "open image");
    incore_free_all();            
    unsigned char map[BLOCK_SIZE] = {0};
    bwrite(INODE_MAP_BLOCK, map);

    struct inode *in = ialloc();
    CTEST_ASSERT(in != NULL, "ialloc returned inode");
    unsigned int orig = in->inode_num;
    in->size = 9999;

    iput(in);


    incore_free_all();
    struct inode *re = iget(orig);
    CTEST_ASSERT(re != NULL, "iget succeeded after iput");
    CTEST_ASSERT(re->size == 9999, "written size matches");
    CTEST_ASSERT(image_close() >= 0, "close image");
}

int main(void) {
    CTEST_VERBOSE(1);
    test_test_free_find_and_set();
    test_inode_incore_find_and_free();
    test_inode_readwrite_round_trip();
    test_inode_alloc_simple_ialloc();
    test_inode_iput_write_on_zero();
    CTEST_RESULTS();
    CTEST_EXIT();
    return 0;
}
