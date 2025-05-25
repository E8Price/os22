#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>        
#include "image.h"
#include "block.h"
#include "inode.h"
#include "dir.h"
#include "pack.h"

#define DIRECTORY_ENTRY_SIZE 32

void mkfs(const char *image_name) {

    image_open((char*)image_name, 1);

    struct inode *in       = ialloc();
    unsigned int  root_ino = in->inode_num;
    unsigned int  blk      = alloc();


    in->flags        = 2;  
    in->size         = 2 * DIRECTORY_ENTRY_SIZE;
    in->block_ptr[0] = blk;


    unsigned char buf[BLOCK_SIZE];
    memset(buf, 0, sizeof(buf));


    write_u16(buf + 0, root_ino);
    strcpy((char*)(buf + 2), ".");


    write_u16(buf + DIRECTORY_ENTRY_SIZE, root_ino);
    strcpy((char*)(buf + DIRECTORY_ENTRY_SIZE + 2), "..");


    bwrite(blk, buf);


    iput(in);
}

struct directory *directory_open(unsigned int inode_num) {
    struct inode *in = iget(inode_num);
    if (!in) return NULL;

    struct directory *d = malloc(sizeof *d);
    if (!d) {
        iput(in);
        return NULL;
    }
    d->inode  = in;
    d->offset = 0;
    return d;
}

int directory_get(struct directory *d, struct directory_entry *ent) {
    if (d->offset >= d->inode->size) return -1;

    unsigned int idx      = d->offset / BLOCK_SIZE;
    unsigned int disk_blk = d->inode->block_ptr[idx];

    unsigned char block[BLOCK_SIZE];
    bread(disk_blk, block);

    unsigned int off = d->offset % BLOCK_SIZE;
    ent->inode_num  = read_u16(block + off);
    memcpy(ent->name, block + off + 2, 16);
    ent->name[15] = '\0';

    d->offset += DIRECTORY_ENTRY_SIZE;
    return 0;
}

void directory_close(struct directory *d) {
    iput(d->inode);
    free(d);
}

void ls(void) {
    struct directory *d = directory_open(0);
    if (!d) return;

    struct directory_entry ent;
    while (directory_get(d, &ent) == 0) {
        printf("%u %s\n", ent.inode_num, ent.name);
    }
    directory_close(d);
}

int path_lookup(const char *path) {
    if (!path) return -1;


    if (path[0] == '\0' || (path[0] == '/' && path[1] == '\0'))
        return 0;

    char *copy = strdup(path);
    if (!copy) return -1;

    int cur_ino = 0;
    char *token = strtok(copy, "/");
    while (token) {
        bool found = false;
        struct directory *d = directory_open(cur_ino);
        if (!d) {
            free(copy);
            return -1;
        }

        struct directory_entry ent;
        while (directory_get(d, &ent) == 0) {
            if (strcmp(ent.name, token) == 0) {
                found   = true;
                cur_ino = ent.inode_num;
                break;
            }
        }

        directory_close(d);
        if (!found) {
            free(copy);
            return -1;
        }
        token = strtok(NULL, "/");
    }

    free(copy);
    return cur_ino;
}
