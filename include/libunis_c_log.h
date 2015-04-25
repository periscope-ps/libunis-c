#ifndef LIBUNIS_C_LOG_H
#define LIBUNIS_C_LOG_H

#include <stdio.h>

#define LOG_MSG_SIZE  4098

//We do not have any other way to enable debugging now. So compile with -DDEBUG
//if you would like to print debug messages on stdout
#define dbg_info(level, ...) \
              do {\
                    if(_log_func != NULL) {\
                        char msg[LOG_MSG_SIZE];\
                        snprintf(msg, LOG_MSG_SIZE, __VA_ARGS__);\
                        _log_func(level, msg);\
                    }\
              } while (0)

//internal variable, do not manipulate directly
//todo: do we really need to set this to NULL???
void (*_log_func)(int level, const char* msg);
// internally used for various log levels, in future, we can also have pluggable log levels
// but I think that is too much. Better to debug
#define ERROR 0
#define WARN 1
#define INFO 2
#define DEBUG 3

void register_log_callback_libunis_c(void (*log_func)(int level, const char* msg));

#endif // LIBUNIS_C_LOG_H
