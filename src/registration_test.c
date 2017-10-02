#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <unis_registration.h>
#include <libunis_c_log.h>

void _logger(int level, const char* msg) {
  fprintf(stderr, "%d: %s", level, msg);
}

int main() {

	register_log_callback_libunis_c(_logger);

	static unis_config config;
	memset(&config, 0, sizeof(unis_config));

	config.name = "IBP BBB";
	config.type = "ibp_server";
	config.endpoint = "https://dlt.crest.iu.edu:9000";
	config.protocol_name = "ibp";
	config.iface = "192.168.1.70";
	config.do_register = 1;
	config.port = 6714;
	config.registration_interval = 5;
	config.refresh_timer = 1;

	config.certfile = "/etc/ibp/dlt-client.pem";
	config.keyfile = "/etc/ibp/dlt-client.key";
	config.use_ssl = 1;
	config.keypass = NULL;
	config.cacerts = NULL;

	unis_init(&config);

	printf("sleeping for 10 seconds\n");

	sleep(10);

	return 0;
}
