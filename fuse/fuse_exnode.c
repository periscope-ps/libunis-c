/**************************************************
*  FUSE: Filesystem in Userspace for UNIS exnodes *	
***************************************************/

#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include "exnode_data.h"

xnode_stack *xnode_st;
int exnode_cnt = 0;

static int xnode_getattr(
			  const char *path, 
			  struct stat *stbuf
			)
{
	int i, res = 0;
	char slash[100] = {0};

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
			memset(slash, 0, 100);
	  		strncpy(slash, "/", 1);
	  		char *file_path = strncat(slash, xnode_st->exnode_data[i]->name, strlen(xnode_st->exnode_data[i]->name));

			if (strncmp(path, file_path, strlen(path)) == 0)
			{
 			  if (strncmp(xnode_st->exnode_data[i]->mode, "directory", 9) == 0)
			  {
				stbuf->st_mode = S_IFDIR | 0755;
				stbuf->st_nlink = 2;
			  } 
			  else if (strncmp(xnode_st->exnode_data[i]->mode, "file", 4) == 0)
			  {
			 	stbuf->st_mode = S_IFREG | 0644;
				stbuf->st_nlink = 1;
				stbuf->st_size = xnode_st->exnode_data[i]->size;
			  }
			  else
			  {
				res = -ENOENT;
			  }
			}
		}
	}

	return res;
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
	int i;

	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	for (i = 0; i < xnode_st->exnode_cnt; i++)
	{
	  filler(buf, xnode_st->exnode_data[i]->name, NULL, 0);
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

	for (i = 0; i < xnode_st->exnode_cnt; i++)
	{
	  memset(slash, 0, 100);
	  strncpy(slash, "/", 1);
	  char *file_path = strncat(slash, xnode_st->exnode_data[i]->name, strlen(xnode_st->exnode_data[i]->name));

	  if (strncmp(path, file_path, strlen(path)) == 0)
	  {
	     valid_path = 1;
	
 	     if ((fi->flags & 3) != O_RDONLY)
	     {
		return -EACCES;
	     }
	     break;
	  }
	}

	if (valid_path == 0)
	{
	  return -ENOENT;
	}

	return 0;
}

/*static int xnode_read(
			 const char *path, 
			 char *buf, size_t size,
			 off_t offset,
		      	 struct fuse_file_info *fi
		       )
{
	size_t len;
	(void) fi;
	if(strcmp(path, hello_path) != 0)
		return -ENOENT;

	len = strlen(str);
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, str + offset, size);
	} else
		size = 0;

	return size;
}*/

static struct fuse_operations xnode_opr = {
						.getattr	= xnode_getattr,
						.readdir	= xnode_readdir,
						.open		= xnode_open,
						//.read		= xnode_read,
					   };

int main(int argc, char *argv[])
{
	int i = 0;
	//exnode **xnode_data = exnode_data;
	xnode_st = retrieve_exnodes(UNIS_URL);

	return fuse_main(argc, argv, &xnode_opr, NULL);
}
