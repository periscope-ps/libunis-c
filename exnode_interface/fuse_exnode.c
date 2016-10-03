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
#include <unis_exnode.h>

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

    //log_msg("\ngetpath : path = %s, exnode = 0x%08x, exnode_path = %s\n", path, xnode, exnode_path);

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

    log_msg("\nxnode_getattr : path = \"%s\", statbuf = 0x%08x\n", path, stbuf);

    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0)
    {
	stbuf->st_mode = S_IFDIR | 0755;
	stbuf->st_nlink = 2;
    }
    else if ((XNODE_DATA->crf != NULL) && (XNODE_DATA->crf->path != NULL) && (strcmp(path, XNODE_DATA->crf->path) == 0))
    {
	stbuf->st_mode = XNODE_DATA->crf->mode;
	stbuf->st_nlink = 1;
	stbuf->st_size = 0;
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
	    if ((strstr(path, res->name) != NULL) && (strncmp(res->mode, "directory", 9) == 0))
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
	    else
	    {
		return -ENOENT;
	    }
	}
	else
	{
	    return -ENOENT;
	}
    }

    log_stat(stbuf);

    return XNODE_SUCCESS;
}


static int xnode_readdir(
			  const char *path, 
			  void *buf, 
			  fuse_fill_dir_t filler,
			  off_t offset, 
			  struct fuse_file_info *fi
			)
{
    (void) offset;
    (void) fi;
    int i, j;
    char *npath, *mpath;
    exnode *res = NULL;
    DIR *dp;

    log_msg("\nxnode_readdir : path=\"%s\", buf=0x%08x, filler=0x%08x, offset=%lld, fi=0x%08x\n", path, buf, filler, offset, fi);

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

    log_fi(fi);

    return XNODE_SUCCESS;
}

static int xnode_open(
		       const char *path, 
		       struct fuse_file_info *fi
		     )
{
    int i = 0, valid_path = 0;
    char slash[100] = {0};
    exnode *res = NULL;

    log_msg("\nxnode_open : path = \"%s\", fi = 0x%08x\n", path, fi);

    if (strcmp(path, "/") == 0)
    {
	valid_path = 1;
    } 
    else if ((XNODE_DATA->crf != NULL) && (XNODE_DATA->crf->path != NULL) && (strcmp(path, XNODE_DATA->crf->path) == 0))
    {
	log_msg("\nxnode_open(crf-path is = \"%s\", fi=0x%08x\n", XNODE_DATA->crf->path, fi);
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

    log_fi(fi);

    return XNODE_SUCCESS;
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
    char *buf2 = NULL, *id = NULL;
    cache *temp = NULL;

    log_msg("\nxnode_read : path = \"%s\", buf = 0x%08x, size = %d, offset = %lld, fi = 0x%08x\n", path, buf, size, offset, fi);
    log_fi(fi);

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
	    XNODE_DATA->buf_addr = NULL;
	    id = get_parent_id(xn->selfRef);
	    temp = XNODE_DATA->exnode_cache;
	    temp = temp->next;

	    while (temp != NULL)
	    {
		if (strcmp(temp->id, id) == 0)
		{
		    log_msg("\nxnode_read : cache id = \"%s\"\n", temp->id);
		    XNODE_DATA->buf_addr = temp->buffer;
		    break;
		}
		temp = temp->next;
	    }
			
	    if (XNODE_DATA->buf_addr == NULL)
	    {
		buf2 = (char *) malloc (xn->size + 1);
		memset(buf2, 0, xn->size + 1);
		fuse_lorsDownload(buf2, xn->selfRef);

		XNODE_DATA->buf_addr = buf2;

		temp = XNODE_DATA->exnode_cache;

		while (temp->next != NULL)
		{
		    temp = temp->next;
		}

		temp->next = (cache *) malloc (1 * sizeof(cache));
		temp = temp->next;
		temp->buffer = buf2;
		temp->size = xn->size + 1;
		temp->id = id;
		temp->next = NULL;
	    }

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

    return XNODE_SUCCESS;
}

static int xnode_mknod(
			 const char *path, 
			 mode_t mode, 
			 dev_t rdev
		      )
{
    int res;
    char *filename = get_filename(path);
    
    log_msg("\nxnode_mknod : path = \"%s\", mode = 0%3o, dev = %lld", path, mode, rdev);

    if (XNODE_DATA->crf != NULL)
    {
       	XNODE_DATA->crf->path = (char *) malloc (strlen(path) + 1);
       	memset(XNODE_DATA->crf->path, 0, strlen(path) + 1);
       	strcpy(XNODE_DATA->crf->path, path);
       	XNODE_DATA->crf->mode = mode;
    }

    return XNODE_SUCCESS;
}

int xnode_write(
		  const char *path, 
		  const char *buf, 
		  size_t size, 
		  off_t offset,
	     	  struct fuse_file_info *fi
	       )
{
    int res = 0, try = 0;
    char *filename = get_filename(path);
    int path_length = (strlen(path) - strlen(filename)) + 1;
    char *path_prefix = (char *) malloc (path_length);
    char **selfRef;
    char *parent_id = NULL;
    xnode_stack *child_st = NULL;
    exnode *child = NULL;

    log_msg("\nxnode_write : path = \"%s\", filename = \"%s\", buf = 0x%08x, size = %d, offset = %lld, fi = 0x%08x\n", path, filename, buf, size, offset, fi);
    log_fi(fi);

    selfRef = (char **) malloc (sizeof(char *));

    memset(path_prefix, 0, path_length);
    strncpy(path_prefix, path, path_length - 1);

    while (try < 10)
    {
       	res = fuse_lorsUpload(path_prefix, filename, (char*) buf, (long) size, selfRef);

       	if (res == 1)
       	{
            log_msg("\txnode_write : !!! lors file upload failed !!! try #%d\n", try);
       	}
       	else
       	{
            break;
       	}
       	try++;
    }

    if (try == 10)
    {
       	return -EBUSY;
    }

    if (*selfRef != NULL)
    {
       	child_st = retrieve_exnodes(*selfRef);

       	if (child_st != NULL)
       	{
 	    child = child_st->exnode_data[0];
       	}
    }

    if (child != NULL)
    {
       	insert_child(child, xnode_st);
    }

    if ((XNODE_DATA->crf != NULL) && (XNODE_DATA->crf->path != NULL))
    {
       	free(XNODE_DATA->crf->path);
       	XNODE_DATA->crf->path = NULL;
    }

    return size;
}

int xnode_mkdir(
		  const char *path, 
		  mode_t mode
	       )
{
    unis_config config; 
    char *upload_dir_id = NULL;
    xnode_stack *dir_st = NULL;
    exnode *upload_dir = NULL;
    int ret = 0;
   
    log_msg("\nxnode_mkdir : path = \"%s\", mode = 0%3o\n", path, mode);

    memset (&config, 0, sizeof(unis_config));
    config.endpoint = UNIS_ENDPOINT;
    config.persistent = 1;

    ret = unis_create_directory(&config, path, &upload_dir_id);

    if (ret == 0)
    {
       	log_msg("\txnode_mkdir : !!! Failed to get the upload directory id from UNIS !!! \n");

       	return XNODE_FAILURE;
    }

    if (upload_dir_id != NULL)
    {
       	dir_st = retrieve_exnodes(upload_dir_id);

       	if (dir_st != NULL)
       	{
 	    upload_dir = dir_st->exnode_data[0];

	    log_msg("\txnode_mkdir : dir name = %s, id = %s \n", upload_dir->name, upload_dir->id);
       	}
    }

    if (upload_dir != NULL)
    {
       	insert_child(upload_dir, xnode_st);
    }

    return XNODE_SUCCESS;
}

int xnode_utime(
		  const char *path, 
		  struct utimbuf *buf
	       )
{
    int res;
    
    log_msg("\nxnode_utime : path = \"%s\", ubuf = 0x%08x\n", path, buf);
    log_utime(buf);

    return XNODE_SUCCESS;
}

int xnode_chmod(
		  const char *path, 
		  mode_t mode
	       )
{   
    log_msg("\nxnode_chmod : path = \"%s\", mode = 0%03o\n", path, mode);

    return XNODE_SUCCESS;
}

int xnode_readlink(
		     const char *path, 
		     char *link, 
		     size_t size
		  )
{
    log_msg("xnode_readlink : path = \"%s\", link = \"%s\", size = %d\n", path, link, size);

    return XNODE_SUCCESS;
}

int xnode_opendir(
		    const char *path, 
		    struct fuse_file_info *fi
		 )
{
    log_msg("\nxnode_opendir : path = \"%s\", fi = 0x%08x\n", path, fi);
    log_fi(fi);

    return XNODE_SUCCESS;
}

int xnode_unlink(
		   const char *path
		)
{
    log_msg("xnode_unlink : path = \"%s\"\n", path);

    return XNODE_SUCCESS;
}

int xnode_rmdir(
		  const char *path
	       )
{
    log_msg("xnode_rmdir : path = \"%s\"\n", path);

    return XNODE_SUCCESS;
}

int xnode_symlink(
		    const char *path, 
		    const char *link
		 )
{
    log_msg("\nxnode_symlink : path = \"%s\", link = \"%s\"\n", path, link);

    return XNODE_SUCCESS;
}

int xnode_rename(
		   const char *path, 
		   const char *npath
		)
{
    log_msg("\nxnode_rename : path = \"%s\", new path = \"%s\"\n", path, npath);

    return XNODE_SUCCESS;
}

int xnode_link(
		 const char *path, 
		 const char *npath
	      )
{
    log_msg("\nxnode_link : path = \"%s\", new path = \"%s\"\n", path, npath);

    return XNODE_SUCCESS;
}

int xnode_chown(
		  const char *path, 
		  uid_t uid, 
		  gid_t gid
	       )
  
{
    log_msg("\nxnode_chown : path = \"%s\", uid = %d, gid = %d\n", path, uid, gid);

    return XNODE_SUCCESS;
}

int xnode_truncate(
		     const char *path, 
		     off_t newsize
		  )
{
    log_msg("\nxnode_truncate : path = \"%s\", new size = %lld\n", path, newsize);

    return XNODE_SUCCESS;
}

int xnode_statfs(
		   const char *path, 
		   struct statvfs *statv
		)
{
    log_msg("\nxnode_statfs : path = \"%s\", statv = 0x%08x\n", path, statv);

    return XNODE_SUCCESS;
}

int xnode_flush(
		  const char *path, 
		  struct fuse_file_info *fi
	       )
{
    log_msg("\nxnode_flush : path = \"%s\", fi = 0x%08x\n", path, fi);
    log_fi(fi);
	
    return XNODE_SUCCESS;
}

int xnode_release(
		    const char *path, 
		    struct fuse_file_info *fi
		 )
{
    log_msg("\nxnode_release : path = \"%s\", fi = 0x%08x\n", path, fi);
    log_fi(fi);

    return XNODE_SUCCESS;
}

int xnode_fsync(
		  const char *path, 
		  int datasync, 
		  struct fuse_file_info *fi
	       )
{
    log_msg("\nxnode_fsync : path = \"%s\", datasync = %d, fi = 0x%08x\n", path, datasync, fi);
    log_fi(fi);

    return XNODE_SUCCESS;    
}

int xnode_releasedir(
		       const char *path, 
		       struct fuse_file_info *fi
		    )
{
    log_msg("\nxnode_releasedir : path = \"%s\", fi = 0x%08x\n", path, fi);
    log_fi(fi);
        
    return XNODE_SUCCESS;
}

int xnode_fsyncdir(
		     const char *path, 
		     int datasync, 
		     struct fuse_file_info *fi
		  )
{
    log_msg("\nxnode_fsyncdir : path = \"%s\", datasync = %d, fi=0x%08x\n", path, datasync, fi);
    log_fi(fi);
    
    return XNODE_SUCCESS;
}

void * xnode_init(
		    struct fuse_conn_info *conn
		 )
{
    log_msg("\nxnode_init :\n");
    log_conn(conn);
    log_fuse_context(fuse_get_context());
    
    return XNODE_DATA;
}

void xnode_destroy(
		     void *xnode_data
		  )
{
    cache *temp1 = NULL, *temp2 = NULL;
    struct exnode_state *state_info; 
    state_info = (struct exnode_state *) xnode_data;

    log_msg("\nxnode_destroy : state_info = 0x%08x\n", state_info);

    temp1 = state_info->exnode_cache;

    while (temp1 != NULL)
    {
	temp2 = temp1;
	temp1 = temp1->next;

	if (temp2->id != NULL)
	{
	    log_msg("\tdeletion id = %s\n", temp2->id);
	    temp2->id = NULL;
	}

	if (temp2->buffer != NULL)
	{
	    free(temp2->buffer);
	    temp2->buffer = NULL;
	}
	
	free(temp2);
    }

    free(state_info->crf);
    free(state_info);
}

int xnode_access(
		   const char *path, 
		   int mask
		)
{
    log_msg("\nxnode_access : path = \"%s\", mask = 0%o\n", path, mask);
    
    return XNODE_SUCCESS;
}

int xnode_ftruncate(
		      const char *path, 
		      off_t offset, 
		      struct fuse_file_info *fi
		   )
{
    log_msg("\nxnode_ftruncate : path = \"%s\", offset = %lld, fi = 0x%08x\n", path, offset, fi);
    log_fi(fi);
    
    return XNODE_SUCCESS;
}


static struct fuse_operations xnode_opr = {
						.getattr	= xnode_getattr,
						.readdir	= xnode_readdir,
						.open		= xnode_open,
						.read		= xnode_read,
						.mknod		= xnode_mknod,
						.write		= xnode_write,
						.mkdir		= xnode_mkdir,
						.utime 		= xnode_utime,
						.chmod		= xnode_chmod,
						// The below system calls are not required for the purpose of this file system and so not implemented
  						.readlink 	= xnode_readlink,
						.opendir 	= xnode_opendir,
  						.unlink 	= xnode_unlink,
  						.rmdir 		= xnode_rmdir,
  						.symlink 	= xnode_symlink,
  						.rename 	= xnode_rename,
  						.link 		= xnode_link,
  						.chown 		= xnode_chown,
  						.truncate 	= xnode_truncate,
  						.statfs 	= xnode_statfs,
  						.flush 		= xnode_flush,
  						.release 	= xnode_release,
  						.fsync 		= xnode_fsync,
    						.releasedir 	= xnode_releasedir,
  						.fsyncdir 	= xnode_fsyncdir,
  						.init 		= xnode_init,
  						.destroy 	= xnode_destroy,
  						.access 	= xnode_access,
  						.ftruncate 	= xnode_ftruncate,
					   };


int main(int argc, char *argv[])
{
    int i = 0, j = 0, k = 0, fuse_status = 0;
    char *npath, *mpath;
    struct exnode_state *state_info;

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
	    xnode_st = create_filesystem_tree(xnode_st);
	}
	else
	{
	    break;
	}
    }
	
    state_info = (struct exnode_state *) malloc (sizeof (struct exnode_state));

    state_info->exnode_cache = (cache *) malloc (1 * sizeof (cache));
    state_info->exnode_cache->id = NULL;
    state_info->exnode_cache->buffer = NULL;
    state_info->exnode_cache->size = 0;
    state_info->exnode_cache->next = NULL;

    state_info->crf = (created_files *) malloc (1 * sizeof (created_files));
    state_info->crf->path = NULL;

    if (state_info == NULL) 
    {
	perror("!!! Memory allocation failed !!!");
	abort();
    }

    state_info->buf_addr = NULL;
    state_info->logfile = log_open();

    fprintf(stderr, "Fuse library version %d.%d\n", FUSE_MAJOR_VERSION, FUSE_MINOR_VERSION);
    fuse_status = fuse_main(argc, argv, &xnode_opr, state_info);

    return fuse_status;
}
