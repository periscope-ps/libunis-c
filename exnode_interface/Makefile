CC = gcc
CFLAGS = -g -O2 -D_LINUX -D_FILE_OFFSET_BITS=64
INCLUDE = -I/usr/local/include -I$(LORS_INC) -I$(XML2_INC) -I/usr/local/include/fuse_exnode
LIBS = -lfuseifc $(XML2_LIB) -lfuse -ljansson -lcurl -lpthread -lunis-c -lwebsockets -luuid -lm -lnsl  
EXTRA_LIBS = 
DEPS = exnode_data.h log.h params.h
OBJ = fuse_exnode.o exnode_interface.o log.o
XML2_INC = /usr/include/libxml2
XML2_LIB = -L/usr/lib/x86_64-linux-gnu -lxml2
LORS_INC = /usr/local/include/lors

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $<

exnode: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBS)

clean:
	rm -f *.o exnode *~
