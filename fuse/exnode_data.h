/**************************************************
*  FUSE: Filesystem in Userspace for UNIS exnodes *	
***************************************************/

#include <stdlib.h>
#include <string.h>

#include <jansson.h>
#include <curl/curl.h>

#define BUFFER_SIZE  (256 * 1024)  /* 256 KB */

#define UNIS_URL   "http://dev.crest.iu.edu:8888/exnodes/570aa83fe779892f09f5a4b4" //"http://dev.crest.iu.edu:8888/exnodes?limit=10"
#define URL_SIZE     256

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

typedef struct
{
  parent *parent;
  char *schema;
  extent **extents;
  struct exnode *exnodes;
  char *name;
  char *selfRef;
  long int created;
  long int modified;
  long int ts;
  char *mode;
  char *id;
  long int size;
}exnode;

typedef struct
{
  exnode *exnode_data[2000];
  int exnode_cnt;
}xnode_stack;

xnode_stack * process_exnode(char *text);
int free_exnodes (xnode_stack *xnode_s);
xnode_stack * retrieve_exnodes (char url[]);

