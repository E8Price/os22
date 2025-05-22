CC     = gcc
CFLAGS = -Wall -Wextra -Werror -I.

LIB_SRCS = image.c block.c free.c inode.c pack.c
LIB_OBJS = image.o block.o free.o inode.o pack.o


libvvsfs.a: $(LIB_OBJS)
	ar rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


testfs: libvvsfs.a testfs.o
	$(CC) $(CFLAGS) -DCTEST_ENABLE -o $@ testfs.o libvvsfs.a


testfs.o: testfs.c
	$(CC) $(CFLAGS) -DCTEST_ENABLE -c testfs.c -o testfs.o


.PHONY: test
test: testfs
	./testfs


.PHONY: clean
clean:
	rm -f *.o libvvsfs.a testfs img
