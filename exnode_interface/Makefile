CC = gcc
CFLAGS = -D_FILE_OFFSET_BITS=64
LIBS = -lfuse -ljansson -lcurl
DEPS = exnode_data.h log.h params.h
OBJ = fuse_exnode.o exnode_interface.o log.o

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

exnode: $(OBJ)
	gcc $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f *.o exnode *~
