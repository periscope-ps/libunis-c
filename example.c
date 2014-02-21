#include <stdlib.h>
#include <stdio.h>

#include "unis_registration.h"

int main()
{
  int i =0;
  unis_ip_port ip_port = { .ip = "10.103.0.11", .port=5006};
  unis_ip_port ip_ports[ ] = {ip_port};
  unis_ip_port_array ip_port_array = {
    .ip_ports = ip_ports,
    .count = 1
  };

  unis_config config = {
    .name = "Ibp Server",
    .type = "ibp_server",
    .endpoint = "http://localhost:8888",
    .ifaces = ip_port_array,
    .do_register = 1,
    .registration_interval = 5,
    .refresh_timer = UNIS_REFRESH_TO
  };

  if(unis_init(config) == 0) {
    printf("success");
  }

  sleep(5);

  char **snames;
  int count;
  if(unis_get_service_access_points("ibp_server", &snames, &count) == 0) {
    for(i=0; i < count; i++) {
      printf("\n%d - %s", i, snames[i]);
    }
  }

  sleep(10000);

  return 0;
}
