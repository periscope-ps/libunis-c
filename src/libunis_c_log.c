// =============================================================================
//  periscope-ps (libunis-c)
//
//  Copyright (c) 2015-2016, Trustees of Indiana University,
//  All rights reserved.
//
//  This software may be modified and distributed under the terms of the BSD
//  license.  See the COPYING file for details.
//
//  This software was created at the Indiana University Center for Research in
//  Extreme Scale Technologies (CREST).
// =============================================================================
#include "libunis_c_log.h"

void register_log_callback_libunis_c(void (*log_func)(int level, const char* msg)) {
    _log_func = log_func;
}
