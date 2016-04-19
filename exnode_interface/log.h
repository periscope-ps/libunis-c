/****************************************
*  FUSE: Logging functionality in fuse  *	
*****************************************/

#ifndef _LOG_H_
#define _LOG_H_
#include <stdio.h>

#define log_struct(st, field, format, typecast) \
  log_msg("    " #field " = " #format "\n", typecast st->field)

FILE *log_open(void);
void log_msg(const char *format, ...);
void log_msg2(const char *format, ...);
int log_error(char *func);

#endif
