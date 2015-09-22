#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <err.h>
#include <jansson.h>
#include <pthread.h>

#include <curl_context.h>
#include <unis_registration.h>
#include <libunis_c_log.h>
#include <compat.h>

static unis_config config;
static curl_context context;
static pthread_t thread;
static char *reg_str = NULL;
static char *service_instance = "\
{\
  \"status\": \"ON\",\
    \"$schema\": \"http://unis.incntre.iu.edu/schema/20140214/service#\",\
    \"serviceType\": \"\",\
    \"name\": \"\",\
    \"accessPoint\": \"\",\
    \"ttl\": 600\
}";

static int unis_make_reg_str(int interval, char *json, char **ret_json);
static void *unis_registration_thread(void *arg);
static json_t * __unis_get_json_listeners();

int unis_init(unis_config* cc) {
    int i = 0;
    service_listener *listener;

    if (cc->name == NULL || !(config.name = strndup(cc->name, strlen(cc->name)))) {
		dbg_info(ERROR, "No UNIS registration name specified!");
		return -1;
    }

    if (cc->type == NULL || !(config.type = strndup(cc->type, strlen(cc->type)))) {
		dbg_info(ERROR, "No UNIS service type specified!");
		return -1;
    }

    if (cc->endpoint == NULL || !(config.endpoint = strndup(cc->endpoint, strlen(cc->endpoint)))) {
		dbg_info(ERROR, "No UNIS endpoint specified!");
		return -1;
    }

    if (cc->protocol_name == NULL || !(config.protocol_name = strndup(cc->protocol_name, strlen(cc->protocol_name)))) {
		dbg_info(ERROR, "No UNIS protocol name specified!");
		return -1;
    }

    if (cc->iface == NULL || !(config.iface = strndup(cc->iface, strlen(cc->iface)))) {
		dbg_info(ERROR, "No interfaces are specified");
		return -1;
    }


    memcpy(&config.loc_info, &cc->loc_info, sizeof(cc->loc_info));
    config.port = cc->port;
    config.do_register = cc->do_register;

    config.refresh_timer = cc->refresh_timer;
    if (config.refresh_timer == UNIS_REFRESH_TO || config.refresh_timer <= 0) {
		dbg_info(WARN, "Refresh time not specified, using default %d\n", UNIS_REFRESH_TO);
		config.refresh_timer = UNIS_REFRESH_TO;
    }

    config.registration_interval = cc->registration_interval;
    if (config.registration_interval == UNIS_REG_INTERVAL || config.registration_interval <= 0) {
		dbg_info(WARN, "Registration interval not specified, using default %d\n", UNIS_REG_INTERVAL);
		config.registration_interval = UNIS_REG_INTERVAL;
    }

    /* we could also start a thread that retrieves and caches everything from UNIS
       for now, every call to the UNIS module will do an active query against the service */
    context.url = config.endpoint;

    context.use_ssl = cc->use_ssl;

    config.listener_count = cc->listener_count;
    if (config.listener_count > 0) {
	config.listeners =
	    malloc(config.listener_count * sizeof(service_listener));
	listener = config.listeners;
	for (i = 0;i<config.listener_count;i++) {
	    if((cc->listeners+i)->is_disabled != 0) {
		listener->protocol_name =
		    strndup((cc->listeners+i)->protocol_name,
			    strlen((cc->listeners+i)->protocol_name));

		if (!(listener->protocol_name)) {
		    dbg_info(ERROR, "No protocol name present for listners");
		    return -1;
		}
		listener->port =
		    strndup((cc->listeners+i)->port,
			    strlen((cc->listeners+i)->port));

		if (!(listener->port)) {
		    dbg_info(ERROR, "No port name present for listners");
		    return -1;
		}
		listener++;
	    }
	}
    }

    // copy paths into context
    context.certfile = Strdup(cc->certfile);
    context.keyfile = Strdup(cc->keyfile);
    context.keypass = Strdup(cc->keypass);
    context.cacerts = Strdup(cc->cacerts);
    context.curl_persist = 0;

    if (init_curl(&context, 0) != 0) {
		dbg_info(ERROR, "Could not start CURL context\n");
		return -1;
    }

    /* do registration at init time
     *      otherwise, we app must call unis_register() */
    if (config.do_register) {
		unis_make_reg_str(config.registration_interval, NULL, &reg_str);

		/* start the registration thread */
		pthread_attr_t attr;
		if(pthread_attr_init(&attr) != 0) {
			dbg_info(ERROR, "can not pthread_attr_init\n");
			return -1;
		}
		if(pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED) != 0) {
			dbg_info(ERROR, "pthread_attr_setdetachstate failed\n");
			return -1;
		}
		if (pthread_create(&thread, &attr, unis_registration_thread, &config) != 0) {
			dbg_info(ERROR, "Could not start UNIS registration thread\n");
			return -1;
		} else {
			dbg_info(DEBUG, "Started thread\n");
		}

    }

    return 0;
}

int unis_query(char *url, char *query, char **ret_str) {
    curl_response *response;
    char *qstr;
    int ret = 0;

    asprintf(&qstr, "%s/%s", url, query);
    dbg_info(DEBUG, "\nQuery: %s\n", qstr);

    curl_get_json_string(&context,
						 qstr,
						 &response);

    if (response && (response->status != 200)) {
		dbg_info(ERROR, "Error querying UNIS: %lu: %s", response->status, response->data);
		ret = -1;
		*ret_str = NULL;
		goto exit;
    }

    if (ret_str && response && response->data) {
		*ret_str = malloc(strlen(response->data) * sizeof(char));
		strncpy(*ret_str, response->data, strlen(response->data));
    }

 exit:
    free(qstr);
    if (response)
		free_curl_response(response);
    return ret;
}


static int unis_make_reg_str(int interval, char *json, char **ret_json) {
    json_t *root;
    json_t *location;
    json_error_t json_err;

    root = json_loads(service_instance, 0, &json_err);
    if (!root) {
		dbg_info(ERROR, "Could not decode service string: %d: %s", json_err.line, json_err.text);
    }

    json_object_set(root, "name", json_string(config.name));
    json_object_set(root, "serviceType", json_string(config.type));
    json_object_set(root, "ttl", json_integer(config.registration_interval));

    location = json_object();
    if (config.loc_info.country && strlen(config.loc_info.country))
		json_object_set(location, "country", json_string(config.loc_info.country));
    if (config.loc_info.street_address && strlen(config.loc_info.street_address))
		json_object_set(location, "streetAddress", json_string(config.loc_info.street_address));
    if (config.loc_info.state && strlen(config.loc_info.state))
		json_object_set(location, "state", json_string(config.loc_info.state));
    if (config.loc_info.institution && strlen(config.loc_info.institution))
		json_object_set(location, "institution", json_string(config.loc_info.institution));
    if (config.loc_info.zipcode && strlen(config.loc_info.zipcode))
		json_object_set(location, "zipcode", json_string(config.loc_info.zipcode));
    if (config.loc_info.email && strlen(config.loc_info.email))
		json_object_set(location, "email", json_string(config.loc_info.email));
    json_object_set(location, "latitude", json_real(config.loc_info.lat));
    json_object_set(location, "longitude", json_real(config.loc_info.lon));

    json_object_set(root, "location", location);

    if (json != NULL) {
		json_t *arg_root;
		arg_root = json_loads(json, 0, &json_err);
		if (!arg_root) {
			dbg_info(ERROR, "Could not decode JSON paramater: %d: %s", json_err.line, json_err.text);
		}
		else {
			/* TODO: merge passed-in JSON with defaults */
		}
    }

    //dbg_info(DEBUG, "%s\n", json_dumps(root, JSON_INDENT(2)));
    *ret_json = json_dumps(root, JSON_COMPACT);

    return 0;
}


//FIX ME: if service is not up when thread has started then somehow
// subsequent call fails
static void *unis_registration_thread(void *arg) {
    dbg_info(DEBUG, "in unis_registration_thread\n");
    struct timeval now;
    unis_config *cfg = (unis_config *)arg;
    curl_response *response;
    json_t *reg_json;
    json_t *listeners;
    json_error_t json_err;
    char *url;
    char *send_str;
    char *sid = NULL;

    asprintf(&url, "%s/%s", cfg->endpoint, "services");

    while (1) {
		dbg_info(DEBUG, "in while loop\n");
		if (reg_str != NULL) {
		        reg_json = json_loads(reg_str, 0, &json_err);
			if (!reg_json) {
				dbg_info(ERROR, "Could not decode registration string: %d: %s\n",
						 json_err.line, json_err.text);
				continue;
			}

			gettimeofday(&now, NULL);
			json_object_set(reg_json, "ts", json_integer(now.tv_sec*1e6 + now.tv_usec));
			if (sid) {
				dbg_info(DEBUG, "\nsid=%s", sid);
				json_object_set(reg_json, "id", json_string(sid));
			}

			/* build the listeners dict on every update as interfaces might come and go
			   this call has the side-effect of setting accessPoint in the root dict */
			listeners = __unis_get_json_listeners(reg_json);
			if (listeners) {
				json_object_set(reg_json, "listeners", listeners);
			}

			send_str = json_dumps(reg_json, JSON_COMPACT);

			/* with valid json, register to UNIS endpoint */
			curl_post_json_string(&context,
					      url,
					      send_str,
					      &response);

			if (response && (response->status != 201)) {
				dbg_info(ERROR, "Error registering to UNIS: %s", response->data);
			}
			/* first time we register, save ID for future updates */
			else if (response && response->data && !sid) {
				json_t *resp;
				json_t *key;
				resp = json_loads(response->data, 0, &json_err);
				if (!resp) {
					dbg_info(ERROR, "Could not decode registration response! %d: %s",
							 json_err.line, json_err.text);
					return NULL;
				}
				key = json_object_get(resp, "id");
				if (key) {
					sid = (char*)json_string_value(key);
				}
			}
			
			if (response) {
			        free_curl_response(response);
			}
		}
		sleep(cfg->registration_interval);
    }

    return NULL;
}

int unis_register_start(unsigned int interval, char *json) {
    return unis_make_reg_str(interval, json, &reg_str);
}

int unis_register_stop() {
    //TODO: do we require mutex to mutate reg_str??
    reg_str = NULL;
    return 0;
}


int unis_get_service_access_points(char *sname, char ***ret_aps, int *num_aps) {
    json_t *json_ret = NULL;
    json_error_t json_err;
    char *query;
    char *ret_str;
    char **aps;
    int num_objs;

    if (!ret_aps || !num_aps)
		return -1;

    asprintf(&query, "services?serviceType=%s", sname);

    unis_query(context.url, query, &ret_str);

    if (ret_str) {
		json_ret = json_loads(ret_str, 0, &json_err);
		if (!json_ret) {
			dbg_info(ERROR, "Could not decode response: %d: %s", json_err.line, json_err.text);
			return -1;
		}
    }

    num_objs = json_array_size(json_ret);
    if (num_objs == 0) {
		ret_aps = NULL;
		*num_aps = 0;
		return 0;
    }
    else {
		aps = (char**)malloc(num_objs*sizeof(char*));
		*num_aps = num_objs;
    }

    /* now we extract the fields we want

       this gets more complicated because updates to UNIS will generate similar
       entries with more recent timestamps.  we should get only the most recent
       entry for a particular UNIS object. more parsing... */

    int i;
    json_t *obj;
    json_t *key;

    //dbg_info("JSON_RESPONSE:\n%s\n", json_dumps(json_ret, JSON_INDENT(2)));

    for (i=0; i<num_objs; i++) {
		obj = json_array_get(json_ret, i);
		key = json_object_get(obj, "accessPoint");

		if (key)
			aps[i] = (char*)json_string_value(key);
    }


    *ret_aps = aps;

    free(query);

    return 0;
}

json_t *get_multiple_listeners(json_t *root, int *status) {
    json_t *listeners;
    char **ips;
    int ip_count, i, j, once = 1;
    listeners = json_array();
    get_all_ips(&ips, &ip_count);
    for(i = 0; i < ip_count; i++) {
		json_t *entry;
		char buf[255];

		if (strchr(ips[i], ':') != NULL) {
			continue;
		}
		else if (!strcmp(ips[i], "127.0.0.1")) {
			continue;
		}
		else {
			if (once) {
				char *ip;
				if (config.iface) {
					/* add listener entry for specified iface */
				    for(j = 0; j<config.listener_count;j++)
				    {
					ip = config.iface;
					snprintf(buf, sizeof(buf), "%s/%d", 
						 ip, atoi(config.listeners[j].port));
					entry = json_object();
					json_object_set(entry, 
							config.listeners[j].protocol_name, 
							json_string(buf));
					json_array_append_new(listeners, entry);
				    }
				}
				else {
					ip = ips[i];
				}
				/* set accessPoint to be first encountered IP, or iface if set */
				snprintf(buf, sizeof(buf), "%s://%s:%d", 
					 config.protocol_name, ip, config.port);
				json_object_set(root, "accessPoint", json_string(buf));
				once = 0;
			}
			/* listen on all non-loopback IPs */
			for(j = 0; j < config.listener_count; j++)
			{
			    snprintf(buf, sizeof(buf), "%s/%d", ips[i], 
				     atoi(config.listeners[j].port));
			    entry = json_object();
			    json_object_set(entry, config.listeners[j].protocol_name, 
					    json_string(buf));
			    json_array_append_new(listeners, entry);
			}
		}
    }

    *status = 0;
    return listeners;
}

static json_t * __unis_get_json_listeners(json_t *root) {
    int status;
    json_t *listeners;
    char **ips;
    int ip_count, i, once = 1;

    /*Alternative path for generating listners with multiple path*/
    if (config.listener_count > 0) {
	listeners = get_multiple_listeners(root, &status);
	if (status == 0) {
	    return listeners;
	}
    }
    listeners = json_array();
    get_all_ips(&ips, &ip_count);
    for(i = 0; i < ip_count; i++) {
		json_t *entry;
		char buf[255];

		if (strchr(ips[i], ':') != NULL) {
			continue;
		}
		else if (!strcmp(ips[i], "127.0.0.1")) {
			continue;
		}
		else {
			if (once) {
				char *ip;
				if (config.iface) {
					/* add listener entry for specified iface */
					ip = config.iface;
					snprintf(buf, sizeof(buf), "%s/%d", ip, config.port);
					entry = json_object();
					json_object_set(entry, "tcp", json_string(buf));
					json_array_append_new(listeners, entry);
				}
				else {
					ip = ips[i];
				}
				/* set accessPoint to be first encountered IP, or iface if set */
				snprintf(buf, sizeof(buf), "%s://%s:%d", config.protocol_name, ip, config.port);
				json_object_set(root, "accessPoint", json_string(buf));
				once = 0;
			}
			/* listen on all non-loopback IPs */
			snprintf(buf, sizeof(buf), "%s/%d", ips[i], config.port);
			entry = json_object();
			json_object_set(entry, "tcp", json_string(buf));
			json_array_append_new(listeners, entry);
		}
    }

    return listeners;
}


/* unis_register_exnode : Post Exnode JSON to UNIS
 * reg_json             : libjansson variable abstracting JSON to POST
 * ret_json          : JSON response from POST
 * 
 */
void unis_register_exnode(json_t *reg_json, char **ret_json){
    char *url;
    char *send_str;
    curl_response *response;
	*ret_json = NULL;

    dbg_info(DEBUG, "Registering files wih UNIS\n");

    /*set the url*/
    asprintf(&url, "%s/%s", config.endpoint,"files");

    /*post the data*/
    send_str = json_dumps(reg_json, JSON_COMPACT);
    curl_post_json_string(&context,
			  url,
			  send_str,
			  &response);

    /*error handling*/
    if (response && (response->status != 201)) {
		dbg_info(ERROR, "Error registering to UNIS: %s", response->data);
		return;
    }

    if (response){
		*ret_json = malloc(sizeof(*response->data));
		memcpy(*ret_json, response->data, sizeof(*response->data));
		free_curl_response(response);
    }
}

void unis_get_exnode(char *filename, char** json_dict){
    char *query;
        
    if(filename == NULL){
		dbg_info(ERROR, "Invalid filename");
		return;
    }
    
    dbg_info(DEBUG, "Retrieving file extents from unis\n");

    asprintf(&query, "%s%s", "files?name=",filename);

    unis_query(context.url, query, json_dict);
    
    dbg_info(DEBUG, "opt : %s \n", *json_dict);

    free(query);
}
