CC = gcc
CFLAGS = -g -O2 -D_LINUX -D_FILE_OFFSET_BITS=64 -DEXNODE_LOG
INCLUDE = -I/home/paven/RA/dlt-lors-master/include -I$(FUSEINC) -I$(LBNINC) -I$(XML2_INC) -I/usr/local/include
FUSEINC = /home/paven/RA/dlt-lors-master/cmdtools
FUSELIB = -L/home/paven/RA/dlt-lors-master/cmdtools -lfuseifc
LIBS = $(FUSELIB) $(XML2_LIB) -lfuse -ljansson -lcurl -lpthread -lxml2 -lunis-c -lwebsockets -luuid -lm -lnsl
EXTRA_LIBS = 
DEPS = exnode_data.h log.h params.h
OBJ = fuse_exnode.o exnode_interface.o log.o
LBNINC = /home/paven/local/include
XML2_INC = /usr/include/libxml2
XML2_LIB = -L/usr/lib

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $<

exnode: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBS)

clean:
	rm -f *.o exnode *~
