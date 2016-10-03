/****************************************
*  FUSE: Logging functionality in fuse  *	
*****************************************/

#include "params.h"
#include "log.h"


FILE *log_open()
{
    FILE *logfile;
    
    logfile = fopen("exnode.log", "w");

    if (logfile == NULL) 
    {
	perror("logfile");
	exit(EXIT_FAILURE);
    }
    
    setvbuf(logfile, NULL, _IOLBF, 0);

    return logfile;
}

void log_msg(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);

    #ifdef EXNODE_LOG
    vfprintf(XNODE_DATA->logfile, format, ap);
    #endif
}

void log_conn(struct fuse_conn_info *conn)
{
    log_msg("\tstruct fuse_conn_info *conn:\n");    
    log_msg("\tmax_write = %d\n", conn->max_write);
    log_msg("\tmax_readahead = %d\n", conn->max_readahead);
}

void log_fuse_context(struct fuse_context *context)
{
    log_msg("\tstruct fuse_context *context:\n");
    log_msg("\tuid = %d\n", context->uid);
    log_msg("\tgid = %d\n", context->gid);
    log_msg("\tpid = %d\n", context->pid);
}

void log_fi (struct fuse_file_info *fi)
{
    log_msg("\tstruct fuse_file_info *fi:\n");
    log_msg("\tflags = 0x%08x\n", fi->flags);
    log_msg("\twritepage = %d\n", fi->writepage);
    log_msg("\tdirect_io = %d\n", fi->direct_io);
    log_msg("\tkeep_cache = %d\n", fi->keep_cache);
    log_msg("\tfh = 0x%016llx\n", fi->fh);
    log_msg("\tlock_owner = 0x%016llx\n", fi->lock_owner);
}

void log_utime(struct utimbuf *buf)
{
    log_msg("\tstruct utimbuf *buf:\n");
    log_msg("\tactime = 0x%08lx\n", buf->actime);
    log_msg("\tmodtime = 0x%08lx\n", buf->modtime);
}

void log_stat(struct stat *si)
{
    log_msg("\tstruct stat *si:\n");
    log_msg("\tst_dev = %lld\n", si->st_dev);
    log_msg("\tst_ino = %lld\n", si->st_ino);
    log_msg("\tst_mode = 0%o\n", si->st_mode);
    log_msg("\tst_nlink = %d\n", si->st_nlink);
    log_msg("\tst_uid = %d\n", si->st_uid);
    log_msg("\tst_gid = %d\n", si->st_gid);
    log_msg("\tst_rdev = %lld\n", si->st_rdev);
    log_msg("\tst_size = %lld\n", si->st_size);
    log_msg("\tst_blksize = %ld\n", si->st_blksize);
    log_msg("\tst_blocks = %lld\n", si->st_blocks);
    log_msg("\tst_atime = 0x%08lx\n", si->st_atime);
    log_msg("\tst_mtime = 0x%08lx\n", si->st_mtime);
    log_msg("\tst_ctime = 0x%08lx\n", si->st_ctime);
}

