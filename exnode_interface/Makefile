CC = gcc
CFLAGS = -g -O2 -D_LINUX -D_FILE_OFFSET_BITS=64 -DEXNODE_LOG
INCLUDE = -I$(FUSEINC) -I$(LBNINC) -I$(XML2_INC) -I$(FUSEINC) -I$(LORSINC) -I/usr/local/include
LIBS = -lfuseifc $(XML2_LIB) -lfuse -ljansson -lcurl -lpthread -lxml2 -lunis-c -lwebsockets -luuid -lm -lnsl
EXTRA_LIBS = 
DEPS = exnode_data.h log.h params.h
OBJ = fuse_exnode.o exnode_interface.o lws_exnode.o log.o
FUSEINC = /usr/local/include/fuse_exnode
LORSINC = /usr/local/include/lors
LBNINC = /usr/local/include
XML2_INC = /usr/include/libxml2
XML2_LIB = -L/usr/lib

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $<

exnode: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBS)

clean:
	rm -f *.o exnode *~
