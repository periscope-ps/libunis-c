#ifndef UNIS_REGISTRATION_H
#define UNIS_REGISTRATION_H

#define UNIS_REFRESH_TO   60
#define UNIS_REG_INTERVAL 720

#include <jansson.h>

typedef struct unis_loc_t {
	char   *country;
	char   *street_address;
	char   *state;
	char   *institution;
	char   *zipcode;
        char   *email;
	float   lat;
	float   lon;
} unis_loc;

typedef struct service_listener_t {
    char         *protocol_name;
    char         *port;
    unsigned int is_disabled;
} service_listener;

typedef struct unis_config_t {
	unis_loc loc_info;
	char *name;
	char *type;
	char *endpoint;
	char *iface;
	char *protocol_name;
	unsigned int port;
	unsigned int do_register;
	unsigned int registration_interval;
	unsigned int refresh_timer;
	unsigned int persistent;
        service_listener *listeners;
        int  listener_count;
	int  use_ssl;     /**< integer 1 or 0 to use ssl */
	char *certfile;   /**< char pointer to certfile */
	char *keyfile;    /**< char pointer to keyfile */
	char *keypass;    /**< char pointer to keypass */
	char *cacerts;    /**< char pointer to cacerts */
} unis_config;

/* public methods */

/*
  should be called to set unis server settings
*/
int unis_init(unis_config* cc);

/* 
   first param is the service name we're looking for
   sets a list of service access points for the caller
*/
int unis_get_service_access_points(char *sname, char ***ret_aps, int *num_aps);

/* allow the app to register its own service attributes */
int unis_register_start(unsigned int interval, char *json);

/* app can also stop registration */
int unis_register_stop();


#endif
