# Project 7: Continuing the File System

## Overview
This project extends our simple file system from Project 6 by adding full inode support
implemented:
- On disk inode layout 
- In-core inode table with iget / iput
- Bitmap-based ialloc to allocate new inodes
- Read/write of inode metadata via pack.c helpers
- Comprehensive tests with ctest.h

