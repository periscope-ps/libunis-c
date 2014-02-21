#ifndef UNIS_REGISTRATION_H
#define UNIS_REGISTRATION_H

#define UNIS_REFRESH_TO   60
#define UNIS_REG_INTERVAL 720

typedef struct unis_ip_port_t {
  char* ip;
  int port;
}unis_ip_port;

typedef struct unis_ip_port_array_t {
  unis_ip_port* ip_ports;
  unsigned int count;
}unis_ip_port_array;

typedef struct unis_config_t {
	char *name;
	char *type;
	char *endpoint;
  unis_ip_port_array ifaces;
  int do_register;
	int registration_interval;
	int refresh_timer;
} unis_config;

const unis_config DEFAULT_UNIS_CONFIG = {
  .name = NULL,
  .type = NULL,
  .endpoint = NULL,
  .ifaces = { .ip_ports = NULL, .count = 0},
  .do_register = 0,
  .registration_interval = UNIS_REG_INTERVAL,
  .refresh_timer = UNIS_REFRESH_TO
};

/* public methods */

/*
   should be called to set unis server settings
*/
int unis_init(unis_config cc);

/* 
   first param is the service name we're looking for
   sets a list of service access points for the caller
*/
int unis_get_service_access_points(char *sname, char ***ret_aps, int *num_aps);

/* allow the app to register its own service attributes */
int unis_register_start(int interval, char *json);

/* app can also stop registration */
int unis_register_stop();

#endif
