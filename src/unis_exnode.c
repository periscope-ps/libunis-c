#include <unis_exnode.h>
#include <curl_context.h>
#include <libunis_c_log.h>
#include <string.h>


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
    (*context)->certfile = strdup(config->certfile);
    (*context)->keyfile = strdup(config->keyfile);
    (*context)->keypass = strdup(config->keypass);
    (*context)->cacerts = strdup(config->cacerts);
    (*context)->curl_persist = 0;

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

	free(context->certfile);
	free(context->keyfile);
	free(context->keypass);
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
		*response_json = malloc(sizeof(response->data)* sizeof(char));
		strncpy(*response_json, response->data, strlen(response->data));
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
		*response_json = malloc(strlen(response->data) * sizeof(char));
		strncpy(*response_json, response->data, strlen(response->data));
    }

	/*free curl response*/
	free_curl_response(response);
	
 free_context:
	unis_curl_free(context);
	
}

/*Create directory on unis*/
void unis_create_directory(unis_config *config, char *dir_path){
}










