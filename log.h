#ifndef LOG_H
#define LOG_H

#include <stdio.h>

#ifndef DEBUG
#define DEBUG 1
#endif

//We do not have any other way to enable debugging now. So compile with -DDEBUG
//if you would like to print debug messages on stdout
#define dbg_info(...) \
              do { if (DEBUG) fprintf(stdout, __VA_ARGS__); } while (0)

#endif // LOG_H
