CC = gcc
CFLAGS = -Wall -D_FILE_OFFSET_BITS=64
LIBS = -lfuse -ljansson -lcurl
DEPS = exnode_data.h
OBJ = fuse_exnode.o exnode_interface.o

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

exnode: $(OBJ)
	gcc $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f *.o exnode *~
