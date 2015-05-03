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
