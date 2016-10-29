/*******************************
Libunis-c websocket Interface  *
********************************/

#include "exnode_data.h"
#include "log.h"

static void KEYB_INTERRUPT_HANDLER (int signum) 
{
    close_connection = 1;
}

static int lws_service_callback(
                             	 struct lws *wsi,
                         	 enum lws_callback_reasons reason, 
				 void *user,
                         	 void *in, 
				 size_t len
			      )
{

    switch (reason) 
    {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
	{
	    // Connection with the server is established successfully

            break;
	}
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
	{
            close_connection = 1;
            break;
	}
        case LWS_CALLBACK_CLOSED:
	{
            close_connection = 1;
            break;
	}
        case LWS_CALLBACK_CLIENT_RECEIVE:
	{
	    xnode_stack *xnd;

	    xnd = process_exnode((char *) in);
	    insert_child(xnd->exnode_data[0], xnode_st);
	    
            break;
	}

        default:
            break;
    }

    return 0;
}

static struct lws_protocols protocols[] = {
    					     {
        					"lws-fuse-protocol",
        					lws_service_callback
    					     },

    					     {NULL, NULL, 0, 0}
					  };


void * lws_fuse (void *arg)
{
    struct sigaction sigact;
    sigact.sa_handler = KEYB_INTERRUPT_HANDLER;
    sigemptyset(&sigact.sa_mask);
    sigaction(SIGINT, &sigact, 0);

    struct lws_client_connect_info connect_info;
    struct lws_context *context = NULL;
    struct lws_context_creation_info info;
    struct lws *wsi = NULL;
    struct lws_protocols protocol;

    memset(&info, 0, sizeof info);
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.iface = NULL;
    info.protocols = protocols;
    info.ssl_cert_filepath = NULL;
    info.ssl_private_key_filepath = NULL;
    info.extensions = NULL;
    info.gid = -1;
    info.uid = -1;
    info.options = 0;

    context = lws_create_context(&info);

    if (context == NULL) {
        return NULL;
    }

    memset(&connect_info, 0, sizeof(connect_info));
    connect_info.context = context;
    connect_info.address = "unis.crest.iu.edu";
    connect_info.port = 8890;
    connect_info.ssl_connection = 0; 
    connect_info.path = "/subscribe/exnodes";
    connect_info.host = "unis.crest.iu.edu:8890";
    connect_info.origin = NULL;
    connect_info.protocol = NULL;
    connect_info.ietf_version_or_minus_one = -1;

    wsi = lws_client_connect_via_info(&connect_info);

    if(wsi == NULL)
    {
        return NULL;
    }

    while (close_connection == 0)
    {
        lws_service(context, 50);
    }

    lws_context_destroy(context);

    return NULL;
}


