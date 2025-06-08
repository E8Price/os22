#ifndef DIR_H
#define DIR_H

#include "inode.h"

#define DIRECTORY_ENTRY_SIZE 32

struct directory_entry {
    unsigned int inode_num;
    char         name[16];
};

struct directory {
    struct inode *inode;
    unsigned int  offset;
};

void mkfs(const char *image_name);

struct directory *directory_open(unsigned int inode_num);
int                directory_get(struct directory *d,
                                 struct directory_entry *ent);
void               directory_close(struct directory *d);

void ls(void);
int  path_lookup(const char *path);

struct inode *namei(char *path);
int           directory_make(char *path);

#endif
