/**************************************************
*  FUSE: Filesystem in Userspace for UNIS exnodes *	
***************************************************/

#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <fuse_interface.h>

#include "exnode_data.h"
#include "log.h"
#include "params.h"

xnode_stack *xnode_st;
int exnode_cnt = 0;


exnode * getpath(
	        char *path,
	        exnode *xnode,
		char *exnode_path
	      )
{
	int i, j;
	exnode *res = NULL;
	char xnode_build_path[100] = {0};
	char xnode_build_path_temp[100] = {0};

	//log_msg("getpath called and path is %s\n", path);

	memset(xnode_build_path, 0, 100);
	strncpy(xnode_build_path, exnode_path, strlen(exnode_path));

	if (strncmp(exnode_path, "/", strlen(exnode_path)) == 0)
	{
		strncat(xnode_build_path, xnode->name, strlen(xnode->name));
	}


	if (strncmp(path, xnode_build_path, strlen(path)) == 0)
	{
		return xnode;
	}
	else if (strncmp(xnode->mode, "directory", 9) == 0)
	{
		strncpy(xnode_build_path_temp, xnode_build_path, strlen(xnode_build_path));

	  	for (j = 0; j < xnode->child_cnt; j++)
		{
			memset(xnode_build_path, 0, 100);
			strncpy(xnode_build_path, xnode_build_path_temp, strlen(xnode_build_path_temp));
			strncat(xnode_build_path, "/", 1);
			strncat(xnode_build_path, xnode->child[j]->name, strlen(xnode->child[j]->name));
			//log_msg("build in progress %s\n", xnode_build_path);

			res = getpath(path, xnode->child[j], xnode_build_path);

			if (res != NULL)
			{
			   return res;
			}
		}
	}
	return NULL;
}


static int xnode_getattr(
			  const char *path, 
			  struct stat *stbuf
			)
{
	int i, j;
	exnode *res = NULL;
	char slash[100] = {0};

	//log_msg("\n --------------- getattr (path is %s) \n", path);

	memset(stbuf, 0, sizeof(struct stat));

	if (strcmp(path, "/") == 0)
	{
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	}
	else
	{
		for (i = 0; i < xnode_st->exnode_cnt; i++)
		{
			res = getpath((char *) path, xnode_st->exnode_data[i], "/");

			if (res != NULL)
			{
			    break;
			}

		}

		if (res != NULL)
		{
		 	if (strncmp(res->mode, "directory", 9) == 0)
	  		{
				stbuf->st_mode = S_IFDIR | 0755;
				stbuf->st_nlink = 2;
	  		}		
	  		else if (strncmp(res->mode, "file", 4) == 0)
	  		{
				stbuf->st_mode = S_IFREG | 0644;
				stbuf->st_nlink = 1;
				stbuf->st_size = res->size;
	  		}
		}
		else
		{
			return -ENOENT;
		}
	}

	return 0;
}


static int xnode_readdir(
			  const char *path, 
			  void *buf, fuse_fill_dir_t filler,
			  off_t offset, 
			  struct fuse_file_info *fi
			)
{
	(void) offset;
	(void) fi;
	int i, j;
        char *npath, *mpath;
	exnode *res = NULL;

	//log_msg("readdir : path is %s \n", path);

	if (strcmp(path, "/") == 0)
	{
	  filler(buf, ".", NULL, 0);
	  filler(buf, "..", NULL, 0);

	  for (i = 0; i < xnode_st->exnode_cnt; i++)
	  {
	    filler(buf, xnode_st->exnode_data[i]->name, NULL, 0);
	  }
	}
	else	
	{
		for (i = 0; i < xnode_st->exnode_cnt; i++)
		{
			//log_msg("--- path is %s\n", path);
			res = getpath((char *) path, xnode_st->exnode_data[i], "/");

			if (res != NULL)
			{
			    break;
			}
		}

		if (res != NULL)
		{
	     	   	for (i = 0; i < res->child_cnt; i++)
	    		{
	        		filler(buf, res->child[i]->name, NULL, 0);
	    		}
		}
		else
		{
	   	 	return -ENOENT;
		}
	}


	return 0;
}

static int xnode_open(
		       const char *path, 
		       struct fuse_file_info *fi
		     )
{
	int i = 0, valid_path = 0;
	char slash[100] = {0};
	exnode *res = NULL;

	if (strcmp(path, "/") == 0)
	{
		valid_path = 1;
	} 
	else
	{
		for (i = 0; i < xnode_st->exnode_cnt; i++)
		{
			res = getpath((char *) path, xnode_st->exnode_data[i], "/");

			if (res != NULL)
			{
			    break;
			}

		}

		if (res != NULL)
		{
			valid_path = 1;

	  	   	if ((fi->flags & 3) != O_RDONLY)
	   		{
			    return -EACCES;
	   		}
		}
	}

	if (valid_path == 0)
	{
		return -ENOENT;
	}

	return 0;
}

static int xnode_read(
			 const char *path, 
			 char *buf, size_t size,
			 off_t offset,
		      	 struct fuse_file_info *fi
		       )
{
	int fd;
	int res, i;
	int flag1 = 0;
	exnode *xn = NULL;
  	char *buf2;

	for (i = 0; i < xnode_st->exnode_cnt; i++)
	{
		xn = getpath((char *) path, xnode_st->exnode_data[i], "/");

		if (xn != NULL)
		{
		    break;
		}
	}

	if (strncmp(xn->mode, "file", 4) == 0)
	{
		if (offset == 0)
		{
			buf2 = (char *) malloc (xn->size + 1);
			memset(buf2, 0, xn->size + 1);
			fuse_lorsDownload(buf2, xn->selfRef);
			XNODE_DATA->buf_addr = buf2;

			memcpy(buf, XNODE_DATA->buf_addr, size);
		}
		else
		{
			if ((offset + size) > xn->size)
			{
			   size = xn->size - offset;
			}

			if (NULL != (XNODE_DATA->buf_addr + offset))
			{
			   memcpy(buf, XNODE_DATA->buf_addr + offset, size);
			}
		}

	      	return size;
	}

	return 0;
}

void print_directory_tree(
			   exnode *xnode
			 )
{
  	int i = 0;

	printf("path is %s\n", xnode->name);

	for (i = 0; i < xnode->child_cnt; i++)
	{
	   print_directory_tree(xnode->child[i]);
	}

}

static struct fuse_operations xnode_opr = {
						.getattr	= xnode_getattr,
						.readdir	= xnode_readdir,
						.open		= xnode_open,
						.read		= xnode_read,
					   };


int main(int argc, char *argv[])
{
	int i = 0, j = 0, k = 0;
	struct exnode_state * state_info;
	char *npath, *mpath;
	xnode_st = retrieve_exnodes(UNIS_URL);

	while (1)
	{
	   k = 0;

	   for (i = 0; i < xnode_st->exnode_cnt; i++)
	   {
	      if (xnode_st->exnode_data[i]->parent != NULL)
	      {
		 k = 1;
	      }
	   }
	  
	   if (k == 1)
	   {
	      printf("created %s\n", xnode_st->exnode_data[0]->name);
	      xnode_st = create_filesystem_tree(xnode_st);
	   }
	   else
	   {
	      break;
	   }
	}
	
	state_info = (struct exnode_state *) malloc (sizeof (struct exnode_state));

	if (state_info == NULL) 
	{
	   perror("main calloc");
	   abort();
    	}

	state_info->buf_addr = NULL;
	

	return fuse_main(argc, argv, &xnode_opr, state_info);
}
