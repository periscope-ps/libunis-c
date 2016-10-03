/**************************************************
*  FUSE: Filesystem in Userspace for UNIS exnodes *	
***************************************************/

#include <stdlib.h>
#include <string.h>

#include <jansson.h>
#include <curl/curl.h>

#define BUF_SIZE  (256 * 1024 * 1024)  /* 256 KB */

#define UNIS_URL  "http://unis.crest.iu.edu:8890/exnodes?limit=10000"   //"http://unis.crest.iu.edu:8890/exnodes/572ae4c8377f97536a9df67b"
//"http://dev.crest.iu.edu:8888/exnodes?limit=50" //"http://dev.crest.iu.edu:8888/exnodes/570aa83fe779892f09f5a4b4"
#define URL_SIZE     256
#define UNIS_ENDPOINT "http://unis.crest.iu.edu:8890/exnodes"

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
