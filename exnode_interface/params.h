/****************************************
*  FUSE: Logging functionality in fuse  *	
*****************************************/

#ifndef _PARAMS_H_
#define _PARAMS_H_

#define _XOPEN_SOURCE 500

#include <limits.h>
#include <stdio.h>
struct exnode_state {
    FILE *logfile;
    char *rootdir;
};
#define XNODE_DATA ((struct exnode_state *) fuse_get_context()->private_data)

#endif
