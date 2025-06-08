#ifndef BLOCK_H
#define BLOCK_H

#define BLOCK_SIZE       4096
#define INODE_MAP_BLOCK  1
#define BLOCK_MAP_BLOCK  2

unsigned char *bread(int block_num, unsigned char *block);
void bwrite(int block_num, unsigned char *block);
int alloc(void);

#endif
