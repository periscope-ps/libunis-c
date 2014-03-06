#include <stdlib.h>
#include <stdio.h>

#include "unis_registration.h"

int main()
{
  int i =0;

  unis_config config = {
    .name = "Ibp Server",
    .type = "ibp_server",
    .endpoint = "http://198.129.50.8:8888",
    .iface = "10.103.0.11",
    .port = 5006,
    .do_register = 1,
    .registration_interval = 5,
    .refresh_timer = UNIS_REFRESH_TO
  };

  if(unis_init(&config) == 0) {
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
