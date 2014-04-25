#include <stdlib.h>
#include <stdio.h>

#include "unis_registration.h"
#include "libunis_c_log.h"

void external_function_call(int level, const char* msg) {
    printf("LOG: %d, %s\n", level, msg);
}

int main()
{
  int i =0;

  unis_config config = {
    .name = "Ibp Server",
    .type = "ibp_server",
    .endpoint = "https://localhost:8888",
    .protocol_name = "ibp",
    .iface = "10.103.0.59",
    .port = 6714,
    .do_register = 1,
    .registration_interval = 5,
    .refresh_timer = UNIS_REFRESH_TO,
    .use_ssl = 1,
    .certfile = "ssl/client.crt",
    .keyfile  = "ssl/client.key",
    .keypass  = "",
    .cacerts  = ""
  };

  register_log_callback_libunis_c(external_function_call);

  if(unis_init(&config) == 0) {
    printf("Success\n");
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
