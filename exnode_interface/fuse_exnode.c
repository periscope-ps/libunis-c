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
#include <unis_exnode.h>

#include "exnode_data.h"
#include "log.h"
#include "params.h"

xnode_stack *xnode_st;
int exnode_cnt = 0;

/*************
getpath : Retrieve and return the exnode data for the current file
**************/
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

	    res = getpath(path, xnode->child[j], xnode_build_path);

	    if (res != NULL)
	    {
		return res;
	    }
	}
    }

    return NULL;
}

/*************
xnode_getattr : Get file attributes similar to stat() 
**************/
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
        if (NULL == xnode_st)
      	    return XNODE_SUCCESS;

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

/*************
xnode_readdir : Return directory entries and is similar to readdir() system call
**************/
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

    log_msg("\nxnode_readdir : path=\"%s\", buf=0x%08x, filler=0x%08x, offset=%lld, fi=0x%08x\n", path, buf, filler, offset, fi);

    if (NULL == xnode_st)
        return XNODE_SUCCESS;

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

/*************
xnode_open : Open a file. But here we are not using file handle and so this function just checks for existence and permissions of the files
**************/
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
	valid_path = 1;		
    }		
    else
    {
    	if (NULL == xnode_st)
            return XNODE_SUCCESS;

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

/*************
xnode_read : Read the file contents into a given buffer and returns the size read
**************/
static int xnode_read(
		       const char *path, // path of the file
		       char *buf,        // pre-allocated buffer to store the contents of the file 
		       size_t size,      // size of the data to store in the buffer
		       off_t offset,     // offset indicating the position in buffer for copying the data 
		       struct fuse_file_info *fi
		     )
{
    int fd;
    int res, i;
    int flag1 = 0;
    exnode *xn = NULL;
    unsigned char *buf1 = NULL;
    unsigned char *buf2 = NULL;
    char *id = NULL;
    char *schema4 = "http://unis.crest.iu.edu/schema/exnode/4/exnode#";
    char *schema6 = "http://unis.crest.iu.edu/schema/exnode/6/exnode#";
    int schema_len = strlen(schema4);
    cache *temp = NULL;

    log_msg("\nxnode_read : path = \"%s\", size = %d, offset = %lld, fi = 0x%08x\n", path, size, offset, fi);
    log_fi(fi);

    if (NULL == xnode_st)
        return XNODE_SUCCESS;

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

	    // Check if the data is already available in the local cache
	    while (temp != NULL)
	    {
		if (strcmp(temp->id, id) == 0)
		{
		    log_msg("\nxnode_read : cache id = \"%s\", size = %ld\n", temp->id, temp->size);
		    XNODE_DATA->buf_addr = temp->buffer;
		    break;
		}
		temp = temp->next;
	    }
	    
	    // If the data is not in the local cache then download it using libdlt		
	    if (XNODE_DATA->buf_addr == NULL)
	    {
		buf2 = (unsigned char *) malloc (xn->size + 1);
		memset(buf2, 0, xn->size + 1);

		if (strncmp(xn->schema, schema4, schema_len) == 0)
		{
		    //fuse_lorsDownload(buf2, xn->selfRef); // Fuse lors implemented but not being used
		    printf("Implementation of fuse lors not used !\n");
		}
		else if (strncmp(xn->schema, schema6, schema_len) == 0)
		{
		    // download the file using libdlt 
		    buf1 = libdlt_download(xn->selfRef);

		    if (buf1 != NULL)
		    {
		        memcpy(buf2, buf1, xn->size);
		    }
		}

		// store the buffer pointer for continuous writing
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

	    if ((NULL != XNODE_DATA->buf_addr) && (NULL != (XNODE_DATA->buf_addr + offset)))
	    {
		memcpy(buf, XNODE_DATA->buf_addr + offset, size);

		if ((offset + size) == xn->size)
		{
		     XNODE_DATA->buf_addr = NULL;
		}
	    }
	    else
	    {
		size = 0;
	    }
	}

	return size;
    }

    return XNODE_SUCCESS;
}

// Commenting xnode_read_buf as it is not supported by older fuse versions. Fuse use xnode_read instead

/*************
xnode_read_buf : Read data from a file into a buffer or store the file handle
**************/
/*
static int xnode_read_buf(
			   const char *path, 
			   struct fuse_bufvec **bufp, // unallocated data structure
			   size_t size,
			   off_t offset,
		      	   struct fuse_file_info *fi
		         )
{
    int fd;
    int res, i;
    int flag1 = 0;
    exnode *xn = NULL;
    unsigned char *buf1 = NULL;
    unsigned char *buf2 = NULL;
    char *id = NULL;
    char *schema4 = "http://unis.crest.iu.edu/schema/exnode/4/exnode#";
    char *schema6 = "http://unis.crest.iu.edu/schema/exnode/6/exnode#";
    int schema_len = strlen(schema4);
    cache *temp = NULL;

    log_msg("\nxnode_read_buf : path = \"%s\", size = %d, offset = %lld, fi = 0x%08x\n", path, size, offset, fi);
    log_fi(fi);

    if (NULL == xnode_st)
        return XNODE_SUCCESS;

    struct fuse_bufvec *src;
    src = (struct fuse_bufvec *) malloc (sizeof(struct fuse_bufvec));

    if (src == NULL)
	return -ENOMEM;

    *src = FUSE_BUFVEC_INIT(size);

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
	    log_msg("\nEnters if statement");
	    XNODE_DATA->buf_addr = NULL;
	    id = get_parent_id(xn->selfRef);
	    temp = XNODE_DATA->exnode_cache;
	    temp = temp->next;

	    while (temp != NULL)
	    {
		if (strcmp(temp->id, id) == 0)
		{
		    log_msg("\nxnode_read : cache id = \"%s\", size = %ld\n", temp->id, temp->size);
		    XNODE_DATA->buf_addr = temp->buffer;
		    break;
		}
		temp = temp->next;
	    }
			
	    if (XNODE_DATA->buf_addr == NULL)
	    {
		buf2 = (unsigned char *) malloc (xn->size + 1);
		memset(buf2, 0, xn->size + 1);

		if (strncmp(xn->schema, schema4, schema_len) == 0)
		{
		    //fuse_lorsDownload(buf2, xn->selfRef);
		    printf("commenting for the moment\n");
		}
		else if (strncmp(xn->schema, schema6, schema_len) == 0)
		{
		    buf1 = libdlt_download(xn->selfRef);

		    if (buf1 != NULL)
		    {
		        memcpy(buf2, buf1, xn->size);
		    }
		}

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

	    src->buf[0].mem = (unsigned char *) malloc (size);
	    memset(src->buf[0].mem, 0, size);
	    memcpy(src->buf[0].mem, XNODE_DATA->buf_addr, size);
	}
	else
	{
	    if ((offset + size) > xn->size)
	    {
		size = xn->size - offset;
		src->buf[0].size = size;
	    }

	    if ((NULL != XNODE_DATA->buf_addr) && (NULL != (XNODE_DATA->buf_addr + offset)))
	    {
	        src->buf[0].mem = (unsigned char *) malloc (size);
	        memset(src->buf[0].mem, 0, size);
	        memcpy(src->buf[0].mem, XNODE_DATA->buf_addr + offset, size);

		if ((offset + size) == xn->size)
		{
		     XNODE_DATA->buf_addr = NULL;
		}
	    }
	    else
	    {
		src->buf[0].size = 0;
	    }
	}

	*bufp = src;

	return XNODE_SUCCESS;
    }

    return XNODE_SUCCESS;
}
*/

/*************
xnode_mknod : Creates a file node
**************/
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

/*************
xnode_write : Write the file data into a buffer if file handle is not used
**************/
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
    char *selfRef;
    char *parent_id = NULL;
    write_cache *temp = NULL;
    cache *temp_cache = NULL;
    
    log_msg("\nxnode_write : path = \"%s\", filename = \"%s\", buf = 0x%08x, size = %d, offset = %lld, fi = 0x%08x\n", path, filename, buf, size, offset, fi);
    log_fi(fi);

    if (offset == 0)
    {
        memset(path_prefix, 0, path_length);
	strncpy(path_prefix, path, path_length - 1);

	temp_cache = XNODE_DATA->exnode_cache;
	temp_cache = temp_cache->next;

	while (temp_cache != NULL)
	{
	    if ((strcmp(temp_cache->path_prefix, path_prefix) == 0) && (strcmp(temp_cache->filename, filename) == 0))
	    {
		log_msg("\nxnode_write : Filesystem does not support writing the existing file to same path !\n");

		return -errno;
	    }
	    temp_cache = temp_cache->next;
	}

	temp = XNODE_DATA->exnode_write_cache;

	while (temp->next != NULL)
	{
            temp = temp->next;
	}

	temp->next = (write_cache *) malloc (1 * sizeof(write_cache));
	temp = temp->next;
	temp->buffer = (unsigned char *) malloc ((long) size);
	memset(temp->buffer, 0, size);
	memcpy(temp->buffer, buf, size);
	temp->size = (long) size;

	temp->filename = filename;
	temp->path_prefix = path_prefix;
	temp->next = NULL;

	XNODE_DATA->buf_addr = temp->buffer;
    }
    else
    {
        if (XNODE_DATA->buf_addr != NULL)
        {
	    // Re-assigning XNODE_DATA->exnode_write_cache as sometimes realloc returns a new pointer
	    temp = XNODE_DATA->exnode_write_cache;

	    while ((temp != NULL) && (temp->buffer != XNODE_DATA->buf_addr))
	    {
		temp = temp->next;
	    }

	    if (temp == NULL)
	    {
	        log_msg("\nxnode_write : temp is NULL\n");

	        return -errno;
	    }
	      
	    int new_size = temp->size + size;
	    temp->size = new_size;
	    XNODE_DATA->buf_addr = (unsigned char *) realloc (XNODE_DATA->buf_addr, new_size);
	    memcpy(XNODE_DATA->buf_addr + offset, buf, size);

	    if (temp->buffer != XNODE_DATA->buf_addr)
	    {
	        temp->buffer = XNODE_DATA->buf_addr;
	    }
        }  
    }
    
    return size;
}

/*************
xnode_write_helper : Helper function for xnode_write that actually push the data into the depot using libdlt. It is not part of the fuse system calls
**************/
char * xnode_write_helper(
		           char *path_prefix,
			   char *filename,
			   unsigned char *buffer,
			   long size
		         )
{
    xnode_stack *child_st = NULL;
    exnode *child = NULL;

    log_msg("xnode_write_helper : path_prefix = %s, filename = %s, size = %ld\n", path_prefix, filename, size);

    // Upload the data using libdlt
    char * selfRef = libdlt_upload(path_prefix, filename, buffer, size);

    if (selfRef != NULL)
    {
       	child_st = retrieve_exnodes(selfRef);

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

    return selfRef;
}

/*************
xnode_mkdir : Creates a directory
**************/
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

/*************
xnode_utime : Used to change the access and modification time of the files. Not required/implemented for this filesystem
**************/
int xnode_utime(
		 const char *path, 
		 struct utimbuf *buf
	       )
{
    log_msg("\nxnode_utime : path = \"%s\", ubuf = 0x%08x\n", path, buf);
    log_utime(buf);

    return XNODE_SUCCESS;
}

/*************
xnode_chmod : Used to change the permission bits of the files. Not required/implemented for this filesystem
**************/
int xnode_chmod(
		 const char *path, 
		 mode_t mode
	       )
{   
    log_msg("\nxnode_chmod : path = \"%s\", mode = 0%03o\n", path, mode);

    return XNODE_SUCCESS;
}

/*************
xnode_readlink : Supports reading of symbolic links. Not required/implemented for this filesystem
**************/
int xnode_readlink(
		    const char *path, 
		    char *link, 
		    size_t size
		  )
{
    log_msg("xnode_readlink : path = \"%s\", link = \"%s\", size = %d\n", path, link, size);

    return XNODE_SUCCESS;
}

/*************
xnode_opendir : Checks if there is open permission for the directory. Not required/implemented for this filesystem
**************/
int xnode_opendir(
		   const char *path, 
		   struct fuse_file_info *fi
		 )
{
    log_msg("\nxnode_opendir : path = \"%s\", fi = 0x%08x\n", path, fi);
    log_fi(fi);

    return XNODE_SUCCESS;
}

/*************
xnode_unlink : Used to remove the given file or symbolic link. Not required/implemented for this filesystem
**************/
int xnode_unlink(
		  const char *path
		)
{
    log_msg("xnode_unlink : path = \"%s\"\n", path);

    return XNODE_SUCCESS;
}

/*************
xnode_rmdir : Used to remove the given directory. Not required/implemented for this filesystem
**************/
int xnode_rmdir(
		 const char *path
	       )
{
    log_msg("xnode_rmdir : path = \"%s\"\n", path);

    return XNODE_SUCCESS;
}

/*************
xnode_symlink : Used to create a symbolic link. Not required/implemented for this filesystem
**************/
int xnode_symlink(
		   const char *path, 
		   const char *link
		 )
{
    log_msg("\nxnode_symlink : path = \"%s\", link = \"%s\"\n", path, link);

    return XNODE_SUCCESS;
}

/*************
xnode_rename : Used to rename a file or directory. Not required/implemented for this filesystem
**************/
int xnode_rename(
		  const char *path, 
		  const char *npath
		)
{
    log_msg("\nxnode_rename : path = \"%s\", new path = \"%s\"\n", path, npath);

    return XNODE_SUCCESS;
}

/*************
xnode_link : Used to create a hard link. Not required/implemented for this filesystem
**************/
int xnode_link(
		const char *path, 
		const char *npath
	      )
{
    log_msg("\nxnode_link : path = \"%s\", new path = \"%s\"\n", path, npath);

    return XNODE_SUCCESS;
}

/*************
xnode_chown : Used to change the owner and group attributes of given file. Not required/implemented for this filesystem
**************/
int xnode_chown(
		 const char *path, 
		 uid_t uid, 
		 gid_t gid
	       )
  
{
    log_msg("\nxnode_chown : path = \"%s\", uid = %d, gid = %d\n", path, uid, gid);

    return XNODE_SUCCESS;
}

/*************
xnode_truncate : Used to truncate the size of the given file. Not required/implemented for this filesystem
**************/
int xnode_truncate(
		    const char *path, 
		    off_t newsize
		  )
{
    log_msg("\nxnode_truncate : path = \"%s\", new size = %lld\n", path, newsize);

    return XNODE_SUCCESS;
}

/*************
xnode_statfs : Return statistics about the filesystem. Not required/implemented for this filesystem
**************/
int xnode_statfs(
		  const char *path, 
		  struct statvfs *statv
		)
{
    log_msg("\nxnode_statfs : path = \"%s\", statv = 0x%08x\n", path, statv);

    return XNODE_SUCCESS;
}

/*************
xnode_flush : Called on each close of a file descriptor and also during the end of xnode_write. Not required/implemented for this filesystem
**************/
int xnode_flush(
		 const char *path, 
		 struct fuse_file_info *fi
	       )
{
    log_msg("\nxnode_flush : path = \"%s\", fi = 0x%08x\n", path, fi);
    log_fi(fi);
	
    return XNODE_SUCCESS;
}

/*************
xnode_release : Used to change the access and modification time of the files. Checks if there are any files in the local cache and push them to depot
**************/
int xnode_release(
		   const char *path, 
		   struct fuse_file_info *fi
		 )
{
    log_msg("\nxnode_release : path = \"%s\", fi = 0x%08x\n", path, fi);
    log_fi(fi);

    cache *temp_cache = NULL;
    write_cache *temp = NULL;
    write_cache *curr = NULL;
    char *selfRef = NULL;
    char *id = NULL;
    
    temp_cache = XNODE_DATA->exnode_cache;
    temp = XNODE_DATA->exnode_write_cache;

    temp = temp->next;
    while (temp != NULL)
    {
        log_msg("\nxnode_release : Call to xnode_write_helper -> filename = \"%s\" and path = \"%s\" and size = %ld\n", temp->filename, temp->path_prefix, temp->size);

        // Upload the data using libdlt
        selfRef = xnode_write_helper(temp->path_prefix, temp->filename, temp->buffer, temp->size);
	id = get_parent_id(selfRef);

	while (temp_cache->next != NULL)
	{
	    temp_cache = temp_cache->next;
	}

	// Copy the values of write_cache to cache and then free the write_cache
	temp_cache->next = (cache *) malloc (1 * sizeof(cache));
	temp_cache = temp_cache->next;
	temp_cache->path_prefix = temp->path_prefix;
	temp_cache->filename = temp->filename;
	temp_cache->buffer = temp->buffer;
	temp_cache->size = temp->size;
	temp_cache->id = id;
	temp_cache->next = NULL;
	
        temp = temp->next;
    }

    temp = XNODE_DATA->exnode_write_cache;

    // Free the write_cache
    while (temp->next != NULL)
    {
        curr = temp->next;
        temp->next = curr->next;
        free(curr);
    }
    XNODE_DATA->buf_addr = NULL;
    
    return XNODE_SUCCESS;
}

/*************
xnode_fsync : Flush information about file to disk. Not required/implemented for this filesystem
**************/
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

/*************
xnode_releasedir : Used to release memory allocated to store information about open directories. Not required/implemented for this filesystem
**************/
int xnode_releasedir(
		      const char *path, 
		      struct fuse_file_info *fi
		    )
{
    log_msg("\nxnode_releasedir : path = \"%s\", fi = 0x%08x\n", path, fi);
    log_fi(fi);
        
    return XNODE_SUCCESS;
}

/*************
xnode_fsyncdir : Flush information about directory to disk. Not required/implemented for this filesystem
**************/
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

/*************
xnode_init : Initializes the file system. Creates threads to handle fuse calls
**************/
void * xnode_init(
		   struct fuse_conn_info *conn
		 )
{
    log_msg("\nxnode_init :\n");

    pthread_t pid;
    pthread_create(&pid, NULL, lws_fuse, NULL);

    log_conn(conn);
    log_fuse_context(fuse_get_context());
    
    return XNODE_DATA;
}

/*************
xnode_destroy : Called when the file system exits. Release the memory allocated for the local cache
**************/
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

/*************
xnode_access : Used to check file access permissions. Not required/implemented for this filesystem
**************/
int xnode_access(
		  const char *path, 
		  int mask
		)
{
    log_msg("\nxnode_access : path = \"%s\", mask = 0%o\n", path, mask);
    
    return XNODE_SUCCESS;
}

/*************
xnode_ftruncate : Truncates or extend the given file. Not required/implemented for this filesystem
**************/
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
						//.read_buf	= xnode_read_buf,
						.read	        = xnode_read,
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

    while (1 && (NULL != xnode_st))
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
    state_info->exnode_cache->path_prefix = NULL;
    state_info->exnode_cache->filename = NULL;
    state_info->exnode_cache->buffer = NULL;
    state_info->exnode_cache->size = 0;
    state_info->exnode_cache->next = NULL;

    state_info->exnode_write_cache = (write_cache *) malloc (1 * sizeof (write_cache));
    state_info->exnode_write_cache->path_prefix = NULL;
    state_info->exnode_write_cache->filename = NULL;
    state_info->exnode_write_cache->buffer = NULL;
    state_info->exnode_write_cache->size = 0;
    state_info->exnode_write_cache->next = NULL;

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
