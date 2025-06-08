#ifndef INODE_H
#define INODE_H

#include <stddef.h>
#include "pack.h"

#define INODE_PTR_COUNT   16
#define INODE_SIZE        64
#define INODE_FIRST_BLOCK 3
#define INODES_PER_BLOCK  (BLOCK_SIZE / INODE_SIZE)
#define MAX_SYS_OPEN_FILES 64

struct inode {
    unsigned int     size;
    unsigned short   owner_id;
    unsigned char    permissions;
    unsigned char    flags;
    unsigned char    link_count;
    unsigned short   block_ptr[INODE_PTR_COUNT];

  
    unsigned int     ref_count;
    unsigned int     inode_num;
};


struct inode *incore_find_free(void);

struct inode *incore_find(unsigned int inode_num);

void         incore_free_all(void);

void         read_inode(struct inode *in, unsigned int inode_num);
void         write_inode(const struct inode *in);


struct inode *iget(unsigned int inode_num);
void          iput(struct inode *in);


struct inode *ialloc(void);

#endif 
