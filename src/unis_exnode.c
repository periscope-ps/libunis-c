#include <unis_exnode.h>
#include <curl_context.h>
#include <libunis_c_log.h>
#include <string.h>
#include <stdlib.h>
#include <jansson.h>

static char * _strdup(const char* str);
char *encode_json(const char *dir, const char *parent_id);
char *parse_json(const char *json_stream, const char *object);
char *_unis_create_directory(curl_context *context, char *dir, char *parent_id);

/* unis_curl_init : Init the curl context and init context
 * unis_config : Unis config to use
 * context : curl context
*/
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
	
	(*context)->url = _strdup(config->endpoint);
    (*context)->use_ssl = config->use_ssl;
    (*context)->certfile = _strdup(config->certfile);
    (*context)->keyfile = _strdup(config->keyfile);
    (*context)->keypass = _strdup(config->keypass);
    (*context)->cacerts = _strdup(config->cacerts);
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

 
/* unis_curl_free : Free the curl context, in case of persistent connection it does curl clean up
 * context : curl context
*/
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

	curl_cleanup(context);

	free(context);
}

/* unis_POST_exnode : POST exnodes to unis
 * unis_config : Unis config to use
 * post_json : JSON dump to push
 * respone_json : return JSON response in allocated buffer, need to freed by user
 */
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


/* unis_GET exnode : get exnodes  from unis in JSON format
 * unis_config : Unis config to use
 * response_json : return JSON string, need to be freed by user
*/
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

/* unis_create_directory : Create directory in directory path.
 * config : unis config to use, make sure curl_persist is set. This function make susequent call to create each directory in the path, To save RRT we need to use persistent connection
 * dir_path : Absolute path from root.
 * return : id for the directory.
*/
char *unis_create_directory(unis_config *config, const char *dir_path){
	
	char *dir;
	char *id;
	char *parent_id = NULL;
	curl_context *context;

	if(dir_path == NULL){
		fprintf(stderr, "Path is null\n");
		return NULL;
	}
	
	if(!config->persistent){
		fprintf(stderr, "create_directory expect curl_persistant flag to be set in unis config \n");
		return NULL;
	}

	if(!unis_curl_init(config, &context)){
		fprintf(stderr, "Failed to init CURL \n");
		return NULL;
	}
	
	char *path = _strdup(dir_path);
	//fprintf(stderr, "Path is %s \n", path);

	// create each directory
	dir = strtok(path, "/");
	while(dir != NULL){
		id = _unis_create_directory(context, dir, parent_id);
		if(id == NULL){
			fprintf(stderr, "Failed to create %s \n", path);
			parent_id = NULL;
			goto free_curl;
		}
		if(parent_id != NULL){
			free(parent_id);
		}
		parent_id = id;
		dir = strtok(NULL, "/");
	}
	
 free_curl:
	unis_curl_free(context);

	free(path);
	return  parent_id;
}


/*_unis_create_directory : Create actual directory
 * context : curl context
 * dir : directory name.
 * parent_dir : id of parent directory
 * return : id of created directory
*/
char *_unis_create_directory(curl_context *context, char *dir, char *parent_id){
	
	if(dir == NULL){
		fprintf(stderr, "Directory is NULL \n");
		return NULL;
	}
	
	if(context == NULL){
		fprintf(stderr, "Curl context is NULL \n");
		return NULL;
	}
	
	char *str_exnode;
	curl_response *response;
	char *id = NULL;

	// encode json
	str_exnode = encode_json(dir, parent_id);
	if(str_exnode == NULL){
		fprintf(stderr, " Exnode JSON encoding failed \n");
		return NULL;
	}

	curl_post_json_string(context,
						  NULL,
						  str_exnode,
						  &response);

	if (response == NULL ||  response->data == NULL) {
		fprintf(stderr, "No data in response \n");
		goto free_exnode;
	}
		
    if (response && (response->status != 201)) {
		fprintf(stderr, "Error while posting data: %s", response->data);
		goto free_response;
    }
	
	id = parse_json(response->data, "id");

 free_response:
	free_curl_response(response);

 free_exnode:	
	free(str_exnode);

	return id;
}

/* parse_json  : Load the JSON stream and find object.
 * json_stream : stream to load.
 * object      : JSON object to find
 * return      : Value of json object
 */
char *parse_json(const char *json_stream, const char *object){

	json_t         *json_ret;
	json_error_t    json_err;
	const char      *ret_str;

	if(json_stream == NULL){
		fprintf(stderr, "JSON stream is NULL \n");
		return NULL;
	}
	
	if(object == NULL){
		fprintf(stderr, "object to find  is NULL \n");
		return NULL;
	}
	
	json_ret = json_loads(json_stream, 0, &json_err);
	if(json_ret == NULL){
		fprintf(stderr, "Could not decode JSON: %d: %s\n", json_err.line, json_err.text);
		return NULL;
	}
	
	json_ret = json_object_get(json_ret, object);
	if(json_ret == NULL){
		fprintf(stderr, "Failed to find %s \n", object);
		return NULL;
	}
	ret_str = json_string_value(json_ret);
	if(ret_str == NULL){
		fprintf(stderr, "Value not found for %s \n", object);
		return NULL;
	}

	return _strdup(ret_str);
}

/* encode_json : encode the exnode in the JSON format n dump it in string
 * dir : directory name
 * parent_id : parent directory id
 * return : JSON dump of exnode
 *
*/
char *encode_json(const char *dir, const char *parent_id){
	
	if(dir == NULL){
		fprintf(stderr, "Invalid directory name \n");
		return NULL;
	}
	
	// create json object
	json_t *json_exnode = json_object();
	json_object_set(json_exnode, "name", json_string(dir));
	json_object_set(json_exnode, "parent", (parent_id == NULL) ? json_null() : json_string(parent_id));
	json_object_set(json_exnode, "created", json_integer(time(NULL)));
	json_object_set(json_exnode, "modified", json_integer(time(NULL)));
	json_object_set(json_exnode, "mode", json_string("directory"));
	json_object_set(json_exnode, "size", json_integer(0));

	return json_dumps(json_exnode, JSON_INDENT(1));
}

static char *_strdup(const char* str){
	
	if(str == NULL){
		return NULL;
	}
	
	int size = strlen(str) + 1;
	char *temp = malloc(size * sizeof(char));
	strncpy(temp, str, size);
	
	return temp;
}
