#ifndef UNIS_REGISTRATION_H
#define UNIS_REGISTRATION_H

#define UNIS_REFRESH_TO   60
#define UNIS_REG_INTERVAL 720

typedef struct unis_config_t {
	char *name;
	char *type;
	char *endpoint;
  char* iface;
  unsigned int port;
  unsigned int do_register;
	unsigned int registration_interval;
	unsigned int refresh_timer;
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
