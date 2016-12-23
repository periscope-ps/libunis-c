/****************************************
*  FUSE: Logging functionality in fuse  *	
*****************************************/

#ifndef _PARAMS_H_
#define _PARAMS_H_

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500
#endif

#define FUSE_USE_VERSION 30
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
    char *path_prefix;
    char *filename;
    unsigned char *buffer;
    long size;
    struct cache *next;
};

struct write_cache
{
    char *path_prefix;
    char *filename;
    unsigned char *buffer;
    long size;
    struct write_cache *next;
};

typedef struct cache cache;
typedef struct write_cache write_cache;

struct exnode_state {
    FILE *logfile;
    char *rootdir;
    unsigned char *buf_addr;
    created_files *crf;
    cache *exnode_cache;
    write_cache *exnode_write_cache;
};
#define XNODE_DATA ((struct exnode_state *) fuse_get_context()->private_data)

#endif
