CC     = gcc
CFLAGS = -Wall -Wextra -Werror -I.

LIB_SRCS = image.c block.c free.c inode.c pack.c dir.c
LIB_OBJS = $(LIB_SRCS:.c=.o)

libvvsfs.a: $(LIB_OBJS)
	ar rcs $@ $^


%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


testfs: libvvsfs.a testfs.o
	$(CC) $(CFLAGS) -DCTEST_ENABLE -o $@ testfs.o libvvsfs.a

testfs.o: testfs.c
	$(CC) $(CFLAGS) -DCTEST_ENABLE -c testfs.c -o testfs.o

ls: libvvsfs.a ls.o
	$(CC) $(CFLAGS) -o $@ ls.o libvvsfs.a

ls.o: ls.c dir.h
	$(CC) $(CFLAGS) -c ls.c -o ls.o

.PHONY: all test clean
all: test ls

test: testfs
	./testfs

clean:
	rm -f *.o libvvsfs.a testfs ls img
