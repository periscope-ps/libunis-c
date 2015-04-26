#include <unis_exnode.h>
#include <curl_context.h>
#include <libunis_c_log.h>
#include <string.h>
#include <stdlib.h>

int unis_curl_init(unis_config *config, curl_context **context){
	
	if(config == NULL){
		dbg_info(ERROR, "Invalid unis config \n");
		return -1;
	}

	if(config->endpoint == NULL){
		dbg_info(ERROR, "Invalid unis endpoint in unis config \n");
		return -1;
	}

	*context = malloc(sizeof(curl_context));
	
	(*context)->url = config->endpoint;
    (*context)->use_ssl = config->use_ssl;
    (*context)->certfile = Strdup(config->certfile);
    (*context)->keyfile = Strdup(config->keyfile);
    (*context)->keypass = Strdup(config->keypass);
    (*context)->cacerts = Strdup(config->cacerts);
    (*context)->curl_persist = 0;
	(*context)->use_cookies = 0;
	(*context)->follow_redirect = 0;

	if (init_curl((*context), 0) != 0) {
		dbg_info(ERROR, "Could not start CURL context\n");
		free((*context));
		return -1;
    }
	
	return 1;
}

/**/
void unis_curl_free(curl_context *context){
	
	if(context == NULL){
		return;
	}
	
	if(context->certfile)
		free(context->certfile);
	
	if(context->keyfile)
		free(context->keyfile);

	if(context->keypass)
		free(context->keypass);
	
	if(context->cacerts)
		free(context->cacerts);
	
	free(context);
}

/*POST exnodes to unis*/
void unis_POST_exnode(unis_config *config, char *post_json, char **response_json){

	curl_context *context;
    curl_response *response;
	
	
	/*Set return variables to NULL*/
	*response_json = NULL;
	
	if(config == NULL){
		fprintf(stderr, "Invalid unis config \n");
		return;
	}

	if(post_json == NULL){
		fprintf(stderr, "No data to POST \n");
		return;
	}

    dbg_info(DEBUG, "Init curl context\n");
	if(!unis_curl_init(config, &context)){
		dbg_info(ERROR, "Failed to init CURL \n");
		return;
	}

    /*post the data*/
    curl_post_json_string(context,
						  NULL,
						  post_json,
						  &response);

    /*error handling*/
    if (response && (response->status != 201)) {
		dbg_info(ERROR, "Error registering to UNIS: %s", response->data);
		goto free_context;
    }
	
	/*copy response data*/
    if (response && response->data) {
		*response_json = (char *)malloc(strlen(response->data) + sizeof(char));
		strncpy(*response_json, response->data, (strlen(response->data) + sizeof(char)));
	}

	/*free curl response*/
	free_curl_response(response);
	
 free_context:
	unis_curl_free(context);
}


/*GET exnodes from unis*/
void unis_GET_exnode(unis_config *config, char **response_json){
	curl_context *context;
    curl_response *response;
	
	/*Set return variables to NULL*/
	*response_json = NULL;

	if(config == NULL){
		fprintf(stderr, "Invalid unis config \n");
		return;
	}


    dbg_info(DEBUG, "Init curl context\n");
	if(!unis_curl_init(config, &context)){
		dbg_info(ERROR, "Failed to init CURL \n");
		return;
	}
    
	curl_get_json_string(context,
						 NULL,
						 &response);

    if (response && (response->status != 200)) {
		dbg_info(ERROR, "Error querying UNIS: %lu: %s", response->status, response->data);
		goto free_context;
    }

    if (response && response->data) {
		*response_json = (char *)malloc(strlen(response->data) + sizeof(char));
		strncpy(*response_json, response->data, (strlen(response->data) + sizeof(char)));
    }

	/*free curl response*/
	free_curl_response(response);
	
 free_context:
	unis_curl_free(context);
	
}

/*Create directory on unis*/
void unis_create_directory(unis_config *config, char *dir_path){
}

/*
void mkdir(char *path, char **dir_id, char *unis_url){
	
	int ret = 0;
	char *dir;
	char *id;
	char *parent_id = NULL;
	*dir_id = NULL;

	if(path == NULL){
		fprintf(stderr, "Path is null\n");
		return;
	}

	if(unis_url == NULL){
		fprintf(stderr, "Unis URL is NULL\n");
		return;
	}
	
	// create each directory
	dir = strtok(path, "/");
	while(dir != NULL){
		create_dir(dir, parent_id, &id, unis_url);
		if(id == NULL){
			fprintf(stderr, "Failed to create %s \n", path);
			return;
		}
		if(parent_id != NULL){
			free(parent_id);
		}
		parent_id = id;
	}

	*dir_id = parent_id;
	
}

void create_dir(char *dir, char *parent_id, char **ret_id, char *unis_url){
	
	*ret_id = NULL;
	
	if(dir == NULL){
		fprintf(stderr, "Directory is NULL \n");
		return;
	}
	
	if(unis_url == NULL){
		fprintf(stderr, "Unis URL is NULL \n");
		return;
	}
	
	char *str_exnode;
	int ret;
	int len;
	char *response;

	// encode json
	str_exnode = encode_json(&json_exnode, dir, parent_id);
	if(str_exnode == NULL){
		fprintf(stderr, " Json encoding failed \n");
		return;
	}

	// POST json to unis
	ret = curl_post_json_string(unis_url, str_exnode, sizeof(str_exnode), &response, &len);
	if(ret != LORS_SUCCESS){
		fprintf(stderr, "Failed to post json to UNIS \n");
		return;
	}
	
	json_loads()
	free(response);

	free(str_exnode);
	


	   
	
}

char * encode_json(const char *dir, const char *parent_id){
	
	if(dir == NULL){
		fprintf(stderr, "Invalid directory name \n");
		return NULL;
	}
	
	// create json object
	json_t *json_exnode = json_object();
	json_object_set(json_exnode, "name", json_string(dir));
	json_object_set(json_exnode, "parent", (parent_id == NULL) ? json_null() : json_string(parent));
	json_object_set(json_exnode, "created", json_integer(time(NULL)));
	json_object_set(json_exnode, "modified", json_integer(time(NULL)));
	json_object_set(json_exnode, "mode", json_string("directory"));
	json_object_set(json_exnode, "size", json_integer(0));

	return json_dumps(json_exnode, JSON_INDENT(1));
}*/








