/****************************************
*  FUSE: Logging functionality in fuse  *	
*****************************************/

#include "params.h"

#include <errno.h>
#include <fuse.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "log.h"

FILE *log_open()
{
    FILE *logfile;
    
    logfile = fopen("exnode.log", "w");
    if (logfile == NULL) {
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

    vfprintf(XNODE_DATA->logfile, format, ap);
}

void log_msg2(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);

    #ifdef LOG
    vfprintf(XNODE_DATA->logfile, format, ap);
    #endif
}

int log_error(char *func)
{
    int ret = -errno;
    
    log_msg("ERROR %s: %s\n", func, strerror(errno));
    
    return ret;
}


