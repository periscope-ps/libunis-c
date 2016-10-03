/****************************************
*  FUSE: Logging functionality in fuse  *	
*****************************************/

#ifndef _LOG_H_
#define _LOG_H_

#include <errno.h>
#include <fuse.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

FILE *log_open(void);
void log_msg(const char *format, ...);
void log_conn(struct fuse_conn_info *conn);
void log_fi(struct fuse_file_info *fi);
void log_fuse_context(struct fuse_context *context);
void log_stat(struct stat *si);
void log_utime(struct utimbuf *buf);

#endif
