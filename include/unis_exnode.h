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
#ifndef UNIS_EXNODE_H
#define UNIS_EXNODE_H

#include <unis_registration.h>

/*POST exnodes to unis*/
void unis_POST_exnode(unis_config *config, char *post_json, char **response_json);

/*GET exnodes from unis*/
void unis_GET_exnode(unis_config *config, char **response_json);

/*Create directory on unis*/
int unis_create_directory(unis_config *config, const char *dir_path, char **key);

#endif
