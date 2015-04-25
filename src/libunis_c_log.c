#include "libunis_c_log.h"

void register_log_callback_libunis_c(void (*log_func)(int level, const char* msg)) {
    _log_func = log_func;
}
