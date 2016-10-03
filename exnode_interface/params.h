/****************************************
*  FUSE: Logging functionality in fuse  *	
*****************************************/

#ifndef _PARAMS_H_
#define _PARAMS_H_

#define _XOPEN_SOURCE 500
#define XNODE_FAILURE -1
#define XNODE_SUCCESS 0

#include <limits.h>
#include <stdio.h>
#include <sys/stat.h>

typedef struct
{
    char *path;
    mode_t mode;
} created_files;

struct cache
{
    char *id;
    char *buffer;
    long size;
    struct cache *next;
};

typedef struct cache cache;

struct exnode_state {
    FILE *logfile;
    char *rootdir;
    char *buf_addr;
    created_files *crf;
    cache *exnode_cache;
};
#define XNODE_DATA ((struct exnode_state *) fuse_get_context()->private_data)

#endif
