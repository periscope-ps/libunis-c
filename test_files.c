#include <stdlib.h>
#include <stdio.h>

#include "unis_registration.h"
#include "libunis_c_log.h"

char *file = "{								\
 \"status\":\"OFF\",							\
 \"$schema\":\"http://unis.incntre.iu.edu/schema/20140909/file#\",	\
 \"description\" : \"Test exnode\",					\
 \"id\" : \"3\",							\
 \"name\" : \"TestFile3\",						\
 \"ttl\"  : 660,							\
 \"size\" : 512,							\
 \"parent\" :\"http://monitor.incntre.iu.edu:9001/files/1\",		\
 \"created\" : 1415288,							\
 \"modified\" : 1415288,						\
 \"extents\" :								\
 [{									\
    \"location\" : {							\
      \"read\"  : \"rdma://b001.bravo.futuregrid.org\",			\
      \"write\" : \"rdma://b001.bravo.futuregrid.org\"			\
    },									\
    \"size\"   : \"256\",						\
    \"offset\" : \"0\",							\
    \"index\"  : \"0\",							\
    \"address\" : \"12345\",						\
    \"keys\"    : [\"8987\",\"5845\"]					\
    },									\
  {									\
  \"location\" : {							\
    \"read\"  : \"rdma://b001.bravo.futuregrid.org\",			\
    \"write\" : \"rdma://b001.bravo.futuregrid.org\"			\
  },									\
  \"size\"   : \"256\",							\
  \"offset\" : \"256\",							\
  \"index\"  : \"1\",							\
  \"address\" : \"23457\",						\
  \"keys\"    : [\"8987\",\"5845\"]					\
  }]									\
}";

void external_function_call(int level, const char* msg) {
    printf("LOG: %d, %s\n", level, msg);
}

int main()
{

    json_t *json_ret;
    json_error_t json_err;
    char *str;
    char *opt_str;

    unis_config config = {
	.name = "RDMA",
	.type = "",
	.endpoint = "http://monitor.incntre.iu.edu:9000",
	.protocol_name = "",
	.iface = "127.0.0.1",
	.port = 0,
	.do_register = 0,
	.registration_interval = 5,
	.refresh_timer = UNIS_REFRESH_TO,
	.use_ssl = 0,
	.certfile = "ssl/client.crt",
	.keyfile  = "ssl/client.key",
	.keypass  = "",
	.cacerts  = "",
	.loc_info = {
	    .country = "US",
	    .zipcode = "18031",
	    .lat = -31.3423,
	    .lon = 121.1231
	}
    };

    register_log_callback_libunis_c(external_function_call);

    if(unis_init(&config) == 0) {
	printf("Success\n");
    }

    unis_get_file_extents("mpi.tar.gz",&str);

    json_ret = json_loads(str, 0, &json_err);
    if (!json_ret) {
	printf("Could not decode response: %d: %s\n", json_err.line, json_err.text);
	printf("Error response : %s\n", str);
	return 1;
    }

    opt_str = json_dumps(json_ret, JSON_INDENT(2));
    if(opt_str != NULL){
	printf("Json dump :\n %s", opt_str);
    }
    
    return 0;
}
