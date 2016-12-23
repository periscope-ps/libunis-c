/**************************************************
*  FUSE: Filesystem in Userspace for UNIS exnodes *	
***************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <unistd.h>
#include <signal.h>
#include <libwebsockets.h>
#include <pthread.h>

#include <jansson.h>
#include <curl/curl.h>
#include <Python.h>

#define BUF_SIZE  (256 * 1024 * 1024)  /* 256 KB */

#define UNIS_URL  "http://dev.crest.iu.edu:8888/exnodes" //"http://unis.crest.iu.edu:8890/exnodes?limit=100"
#define URL_SIZE  256
#define UNIS_ENDPOINT "http://dev.crest.iu.edu:8888/exnodes" //"http://unis.crest.iu.edu:8890/exnodes"

static int close_connection = 0;

typedef struct
{
  char *href;
  char *rel;
}parent;

typedef struct
{
  char *read;
  char *write;  
  char *manage;
}mapping;

typedef struct
{
  char *selfRef;
  parent *parent;
  char *lifetimes;
  long int ts;
  mapping *mapping;
  char *location;
  char *offset;
  char *schema;
  char *id;
  long int size;
}extent;

struct exnode
{
  parent *parent;
  char *schema;
  extent **extents;
  struct exnode *child[10000];
  int child_cnt;
  char *name;
  char *selfRef;
  long int created;
  long int modified;
  long int ts;
  char *mode;
  char *id;
  long int size;
};

typedef struct exnode exnode;

typedef struct
{
  exnode *exnode_data[10000];
  int exnode_cnt;
}xnode_stack;

extern xnode_stack *xnode_st;

xnode_stack * process_exnode(char *text);
int free_exnodes (xnode_stack *xnode_s);
char * get_parent_id(char parent_href[]);
char * get_filename(const char *path);
char * get_path_prefix(const char *path, char *filename);
exnode * _retrieve_parent (char *parent_id, exnode *parent_node);
exnode * retrieve_parent (char *parent_id, xnode_stack *xnode_st);
void insert_child (exnode *child, xnode_stack *xnode_st);
xnode_stack * retrieve_exnodes (char url[]);
xnode_stack * create_filesystem_tree(xnode_stack *);
unsigned char * libdlt_download(char *selfRef);
char * libdlt_upload(char *path_prefix, char *filename, const char *buf, long size);

static int lws_service_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
void * lws_fuse (void *message);
