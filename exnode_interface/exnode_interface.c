/****************************
Libunis-c Exnode Interface  *
*****************************/

#include "exnode_data.h"
#include "log.h"

struct write_result
{
    char *data;
    int pos;
};

static size_t write_response(
			      void *ptr, 
			      size_t size, 
			      size_t nmemb, 
			      void *stream
			    )
{
    struct write_result *result = (struct write_result *)stream;

    if(result->pos + size * nmemb >= BUF_SIZE - 1)
    {
        fprintf(stderr, "error: too small buffer\n");
        return 0;
    }

    memcpy(result->data + result->pos, ptr, size * nmemb);
    result->pos += size * nmemb;

    return size * nmemb;
}

static char *request(
		      const char *url
		    )
{
    CURL *curl = NULL;
    CURLcode status;
    struct curl_slist *headers = NULL;
    char *data = NULL;
    long code;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if(!curl)
        goto error;

    data = malloc(BUF_SIZE);
    if(!data)
        goto error;

    struct write_result write_result = {
        .data = data,
        .pos = 0
    };

    curl_easy_setopt(curl, CURLOPT_URL, url);

    headers = curl_slist_append(headers, "User-Agent: Jansson");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_response);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_result);

    status = curl_easy_perform(curl);
    if(status != 0)
    {
        fprintf(stderr, "error: unable to request data from %s:\n", url);
        fprintf(stderr, "%s\n", curl_easy_strerror(status));
        goto error;
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    if(code != 200)
    {
        fprintf(stderr, "error: server responded with code %ld\n", code);
        goto error;
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    curl_global_cleanup();

    data[write_result.pos] = '\0';

    return data;

error:
    if(NULL != data)
    {
        free(data);
	data = NULL;
    }

    if(curl)
        curl_easy_cleanup(curl);
    if(headers)
        curl_slist_free_all(headers);
    curl_global_cleanup();

    return NULL;
}

xnode_stack * process_exnode(
			      char *text
			    )
{
    int i = 0;
    int index = 0;
    exnode *xnode = NULL;
    json_t *arr_data = NULL;
    json_t *root = NULL;
    json_error_t error;
    parent *parentt = NULL;
    json_t *p = NULL;
    json_t *href = NULL;
    json_t *rel = NULL;
    json_t *schema = NULL;
    json_t *name = NULL;
    json_t *selfRef = NULL;
    json_t *created = NULL;
    json_t *modified = NULL;
    json_t *ts = NULL;
    json_t *mode = NULL;
    json_t *id = NULL;
    json_t *size = NULL;
    json_t *extents = NULL;
    xnode_stack *xnode_st = NULL;

    xnode_st = (xnode_stack *) malloc (1 * sizeof(xnode_stack));
    memset(xnode_st, 0, sizeof(xnode_stack));

    root = json_loads(text, 0, &error);

    if(!root)
    {
        fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
        return NULL;
    }


    if(!json_is_array(root))
    {
	arr_data = json_array();
	json_array_append_new(arr_data, root);
	root = arr_data;
    }

    for(i = 0; i < json_array_size(root); i++)
    {
        json_t *data;

	xnode = (exnode *) malloc (1 * sizeof(exnode));
    	parentt = (parent *) malloc (1 * sizeof(parent));

        data = json_array_get(root, i);
        if(!json_is_object(data))
        {
            fprintf(stderr, "error: data [%d] is not an object\n", (int)(i + 1));
            json_decref(root);
            return NULL;
        }

 	json_t *data_value;

    	data_value = json_object_get(data, "data");

	if (NULL != data_value)
	{
	    data = data_value;
	}

        p = json_object_get(data, "parent");
        if(!json_is_object(p))
        {
	    if (json_is_null(p))
	    {
	    	parentt = NULL;
		xnode->parent = parentt;
	    }
	    else
	    {
            	fprintf(stderr, "error: [%d] parent is not a json object\n", (int)(i + 1));
            	return NULL;
	    }
        }
	else
	{
	   href = json_object_get(p, "href");
	   rel = json_object_get(p, "rel");

	   parentt->href = (char*) malloc ((strlen(json_string_value(href))+1) * sizeof(char));
	   memset(parentt->href, 0, strlen(parentt->href));
	   strcpy(parentt->href, json_string_value(href));

	   parentt->rel = (char*) malloc ((strlen(json_string_value(rel))+1) * sizeof(char));
	   memset(parentt->rel, 0, strlen(parentt->rel));
	   strcpy(parentt->rel, json_string_value(rel));

	   xnode->parent = parentt;
	}

        schema = json_object_get(data, "$schema");
        if (!json_is_string(schema))
        {
	    schema = json_object_get(data, "\\$schema"); // websocket subscription updates used "\\$schema" for schema
	    if (!json_is_string(schema))
	    {
                fprintf(stderr, "error: [%d] schema is not a string\n", (int)(i + 1));
                json_decref(root);
            	return NULL;
	    }
        }
	xnode->schema = (char*) malloc ((strlen(json_string_value(schema))+1) * sizeof(char));
	memset(xnode->schema, 0, strlen(xnode->schema));
	strcpy(xnode->schema, json_string_value(schema));

        name = json_object_get(data, "name");
        if(!json_is_string(name))
        {
            fprintf(stderr, "error: [%d] name is not a string\n", (int)(i + 1));
            json_decref(root);
            return NULL;
        }
	xnode->name = (char*) malloc ((strlen(json_string_value(name))+1) * sizeof(char));
	memset(xnode->name, 0, strlen(xnode->name));
	strcpy(xnode->name, json_string_value(name));

        selfRef = json_object_get(data, "selfRef");
        if(!json_is_string(selfRef))
        {
            fprintf(stderr, "error: [%d] selfRef is not a string\n", (int)(i + 1));
            json_decref(root);
            return NULL;
        }
	xnode->selfRef = (char*) malloc ((strlen(json_string_value(selfRef))+1) * sizeof(char));
	memset(xnode->selfRef, 0, strlen(xnode->selfRef));
	strcpy(xnode->selfRef, json_string_value(selfRef));

        mode = json_object_get(data, "mode");
        if(!json_is_string(mode))
        {
            fprintf(stderr, "error: [%d] mode is not a string\n", (int)(i + 1));
            json_decref(root);
            return NULL;
        }
	xnode->mode = (char*) malloc ((strlen(json_string_value(mode))+1) * sizeof(char));
	memset(xnode->mode, 0, strlen(xnode->mode));
	strcpy(xnode->mode, json_string_value(mode));

        id = json_object_get(data, "id");
        if(!json_is_string(id))
        {
            fprintf(stderr, "error: [%d] id is not a string\n", (int)(i + 1));
            json_decref(root);
            return NULL;
        }
	xnode->id = (char*) malloc ((strlen(json_string_value(id))+1) * sizeof(char));
	memset(xnode->id, 0, strlen(xnode->id));
	strcpy(xnode->id, json_string_value(id));

        size = json_object_get(data, "size");
        if(!json_is_number(size))
        {
            fprintf(stderr, "error: [%d] size is not an integer\n", (int)(i + 1));
            json_decref(root);
            return NULL;
        }
	xnode->size = json_integer_value(size);

	xnode->child_cnt = 0;
	xnode_st->exnode_data[index] = xnode;
 	index++;
    }
    xnode_st->exnode_cnt = index;

    json_decref(root);

    return xnode_st;
}  

int free_exnodes (
		   xnode_stack *xs
		 )
{
    int i = 0;

    for (i = 0; i < xs->exnode_cnt; i++)
    {
    	if (NULL != xs->exnode_data[i])
    	{
      	    free(xs->exnode_data[i]);
      	    xs->exnode_data[i] = NULL;
    	}
    }

    if (NULL != xs)
    {
        free(xs);
        xs = NULL;
    }

    return 0;
}

xnode_stack * retrieve_exnodes(
				char url[]
			      )
{
    char *text = NULL;
    int xnode_cnt = 0;
    xnode_stack *xnode_st = NULL;

    text = request(url);

    if (!text || !strncmp(text, "[]", 2))
    {
        return NULL;
    }

    xnode_st = process_exnode(text);

    return xnode_st;
} 

void insert_exnode(
		    exnode *parent_xnode, 
		    exnode *child_exnode
		  )
{
    if (NULL != parent_xnode)
    {
       parent_xnode->child[parent_xnode->child_cnt] = (struct exnode *) child_exnode;
       parent_xnode->child_cnt++;
    }
}

char * get_parent_id(
		      char parent_href[]
		    )
{
   char *parent_location;
   const char s[2] = "/";
   char *token, *token2;
   
   parent_location = (char *) malloc (200);
   memset(parent_location, 0, 200);
   strncpy(parent_location, parent_href, strlen(parent_href));

   token = strtok(parent_location, s);
   
   while (token != NULL) 
   {    
      token = strtok(NULL, s);
      if (NULL != token)
      {
	 token2 = token;
      }
   }
   
   return token2;
}

char * get_filename(
		      const char *path
		    )
{
   char *file_path;
   const char s[2] = "/";
   char *token = NULL, *token2 = NULL, *token3 = NULL;
   
   file_path = (char *) malloc (200);
   memset(file_path, 0, 200);
   strncpy(file_path, path, strlen(path));

   token = strtok(file_path, s);
   token3 = token;

   while (token != NULL) 
   {    
      token = strtok(NULL, s);

      if (NULL != token)
      {
	 token2 = token;
      }
   }

   if (token2 == NULL)
     token2 = token3;
   
   return token2;
}

char * get_path_prefix(
		         const char *path,
			 char *filename
		      )
{
   char *file_path;
   char *token, *token2;
   
   file_path = (char *) malloc (200);
   memset(file_path, 0, 200);
   strncpy(file_path, path, strlen(path));

   token = strtok(file_path, filename);
   
   while (token != NULL) 
   {    
      token = strtok(NULL, filename);
      if (NULL != token)
      {
	 token2 = token;
      }
   }
   
   return token;
}

exnode * find_parent (
		        exnode *xnode,
		        char *parent_id
	 	     )
{
    int i = 0;
    exnode *xnod = NULL;

    if (strncmp(parent_id, xnode->id, strlen(parent_id)) == 0)
    {
	return xnode;
    }

    for (i = 0; i < xnode->child_cnt; i++)
    {
	xnod = find_parent (xnode->child[i], parent_id);

	if (xnod != NULL)
	{
	    return xnod;
	}
    }

    return NULL;
}

exnode * _retrieve_parent (
		             char *parent_id,
		             exnode *parent_node
		          )
{
    int i = 0;
    exnode *parent = NULL;

    for (i = 0; i < parent_node->child_cnt; i++)
    {
	if (strcmp(parent_id, parent_node->child[i]->id) == 0)
	{
	    return parent_node->child[i];
	}

	parent = _retrieve_parent(parent_id, parent_node->child[i]);

	if (parent != NULL)
	{
	    return parent;
	}
    }

    return parent;
}

exnode * retrieve_parent (
	 	            char *parent_id,
		            xnode_stack *xnode_st
		         )
{
    int i, j;
    exnode *parent = NULL;

    for (i = 0; i < xnode_st->exnode_cnt; i++)
    {
	if (strcmp(parent_id, xnode_st->exnode_data[i]->id) == 0)
	{
	    return xnode_st->exnode_data[i];
	}
 
	for (j = 0; j < xnode_st->exnode_data[i]->child_cnt; j++)
	{
	    if (strcmp(parent_id, xnode_st->exnode_data[i]->child[j]->id) == 0)
	    {
	        return xnode_st->exnode_data[i]->child[j];
	    }

	    parent = _retrieve_parent(parent_id, xnode_st->exnode_data[i]->child[j]);

	    if (parent != NULL)
	    {
	        return parent;
	    }
	}
    }

    return parent;
}

void insert_child (
		    exnode *child,
		    xnode_stack *xnode_st
		 )
{
    int i = 0, ret = 0;
    char *parentRef = NULL;
    char *parent_id = NULL;
    exnode *parent = NULL, *x_temp = NULL;
    xnode_stack *xs_temp = NULL;

    if (child->parent != NULL)
    {
       parentRef = child->parent->href;
    }

    if (parentRef == NULL)
    {
       for (i = 0; i < xnode_st->exnode_cnt; i++)
       {
	  if (strcmp(xnode_st->exnode_data[i]->id, child->id) == 0)
	  {
	     return;
	  }
       }

       xnode_st->exnode_data[xnode_st->exnode_cnt] = child;
       xnode_st->exnode_cnt++;

       return;
    }

    parent_id = get_parent_id(parentRef);
    parent = retrieve_parent(parent_id, xnode_st);

    if (parent == NULL)
    {
       xs_temp = retrieve_exnodes(parentRef);
       x_temp = xs_temp->exnode_data[0];

       x_temp->child[x_temp->child_cnt] = child;
       x_temp->child_cnt++;

       insert_child(x_temp, xnode_st);
    }
    else
    {
       for (i = 0; i < parent->child_cnt; i++)
       {
	  if (strcmp(parent->child[i]->id, child->id) == 0)
	  {
	     return;
	  }
       }

       parent->child[parent->child_cnt] = child;
       parent->child_cnt++;
    }

    return;
}

void print_directory_tree(
			   exnode *xnode
			 )
{
  	int i = 0;

	printf("path is %s\n", xnode->name);

	for (i = 0; i < xnode->child_cnt; i++)
	{
	   print_directory_tree(xnode->child[i]);
	}

}

xnode_stack * create_filesystem_tree(
				      xnode_stack *xnode_s
				    )
{
    int i = 0, j = 0, index = 0, parent_found = 0;
    xnode_stack *xnode_parent;
    char *parent_id = NULL;
    exnode *xnod = NULL;

    xnode_parent = (xnode_stack *) malloc (1 * sizeof(xnode_stack));

    for (i = 0; i < xnode_s->exnode_cnt; i++)
    {
       xnod = NULL;

       if ((xnode_s->exnode_data[i]->parent != NULL) && (strcmp(xnode_s->exnode_data[i]->parent->href, "") != 0))
       {
          parent_id = get_parent_id(xnode_s->exnode_data[i]->parent->href);
          parent_found = 0;


	  if (NULL != parent_id)
	  {
             for (j = 0; j < index; j++)
             {
	        xnod = find_parent(xnode_parent->exnode_data[j], parent_id);

	        if (NULL != xnod)
	        {
		   break;
	        }
             }
	  }

	  if (NULL == xnod)
	  {
	     for (j = i+1; j < xnode_s->exnode_cnt; j++)
	     {
	        xnod = find_parent(xnode_s->exnode_data[j], parent_id);

	        if (NULL != xnod)
	        {
		   break;
	        }
	     }
          }
	     
	  if (NULL != xnod)
	  {
	     insert_exnode(xnod, xnode_s->exnode_data[i]);
	     parent_found = 1;
	  }
          else
          {
	     xnode_stack *xs = retrieve_exnodes(xnode_s->exnode_data[i]->parent->href);
	
	     if (NULL != xs)
	     {
	        xnode_parent->exnode_data[index] = xs->exnode_data[0];
	        insert_exnode(xnode_parent->exnode_data[index], xnode_s->exnode_data[i]);
	        index++;
	     }
	     else
	     {
		xnode_parent->exnode_data[index] = xnode_s->exnode_data[i];
		xnode_s->exnode_data[i]->parent = NULL;
	        index++;
	     }
          }
       }
       else
       {
	  xnode_s->exnode_data[i]->parent = NULL;

	  xnode_parent->exnode_data[index] = xnode_s->exnode_data[i];
          index++;
       }
    }
    xnode_parent->exnode_cnt = index;

    return xnode_parent;
}

unsigned char * libdlt_download(char *selfRef)
{
    log_msg("\nlibdlt_upload : selfRef = %s", selfRef);

    Py_Initialize();

    PyObject* main = PyImport_AddModule("__main__");
    PyObject* argparse = PyImport_ImportModule("argparse");
    PyObject* json = PyImport_ImportModule("json");
    PyObject* libdlt = PyImport_ImportModule("libdlt");
    PyModule_AddObject(main, "argparse", argparse);
    PyModule_AddObject(main, "json", json);
    PyModule_AddObject(main, "libdlt", libdlt);
    PyObject* globalDictionary = PyModule_GetDict(main);
    PyObject* localDictionary = PyDict_New();

    PyDict_SetItemString(globalDictionary, "selfRef", PyUnicode_FromString(selfRef));
    PyDict_SetItemString(globalDictionary, "UNIS_URL", PyUnicode_FromString("http://dev.crest.iu.edu:8888"));
    PyDict_SetItemString(globalDictionary,
			 "DEPOTS",
			 PyUnicode_FromString("{\"ceph://um-mon01.osris.org\": {\"clustername\": \"osiris\","
                                                                                "\"config\": \"/etc/ceph/osiris.conf\","
                                                                                "\"pool\": \"dlt\","
                                                                                "\"crush_map\": \"None\"}," 
                                                "\"ibp://ibp2.crest.iu.edu:6714\": {\"duration\": 2592000}}"));

    PyRun_String(
		  "def main():\n"
		  "    bs = '5m'\n"
		  "    depots = None\n"

		  "    if DEPOTS:\n"
		  "        try:\n"
		  "            depots = json.loads(DEPOTS)\n"
		  "        except Exception as e:\n"
		  "            print (\"ERROR: Could not read depot file: {}\".format(e))\n"
		  "            exit(1)\n"

		  "    host = UNIS_URL\n"
		  "    sess = libdlt.Session(host, bs=bs, depots=depots, **{\"viz_url\": None})\n"
		  "    sess.use_buffer = True\n"
		  "    xfer = sess.download\n"
        
		  "    files = [selfRef]\n"
		  "    for f in files:\n"
		  "        diff, res = xfer(f, None)\n"
		  "        print (\"{0} ({1} {2:.2f} MB/s) {3}\".format(res.name, res.size, res.size/1e6/diff, res.selfRef))\n"

		  "    return sess.data_buffer\n"

 
		  "if __name__ == \"__main__\":\n"
		  "    data = main()\n"
		  "    download_data = bytes(data)\n",
	 	  Py_file_input, globalDictionary, localDictionary);

    PyObject* data = PyDict_GetItemString(localDictionary, "download_data");
    unsigned char *res_buffer = PyBytes_AsString(data);

    Py_Finalize();

    return res_buffer;
}

char * libdlt_upload(char *path_prefix, char *filename, const char *buf, long size)
{
    log_msg("\nlibdlt_upload : path = %s, filename = %s, size = %ld", path_prefix, filename, size);

    Py_Initialize();

    PyObject* main = PyImport_AddModule("__main__");
    PyObject* argparse = PyImport_ImportModule("argparse");
    PyObject* json = PyImport_ImportModule("json");
    PyObject* libdlt = PyImport_ImportModule("libdlt");
    PyModule_AddObject(main, "argparse", argparse);
    PyModule_AddObject(main, "json", json);
    PyModule_AddObject(main, "libdlt", libdlt);
    PyObject* globalDictionary = PyModule_GetDict(main);
    PyObject* localDictionary = PyDict_New();

    PyDict_SetItemString(globalDictionary, "data", PyBytes_FromStringAndSize(buf, PyLong_AsSsize_t(PyLong_FromLong(size))));
    PyDict_SetItemString(globalDictionary, "data_size", PyLong_FromLong(size));    
    PyDict_SetItemString(globalDictionary, "filename", PyUnicode_FromString(filename));
    PyDict_SetItemString(globalDictionary, "output", PyUnicode_FromString(path_prefix));
    PyDict_SetItemString(globalDictionary, "UNIS_URL", PyUnicode_FromString("http://dev.crest.iu.edu:8888"));
    PyDict_SetItemString(globalDictionary,
			 "DEPOTS",
			 PyUnicode_FromString("{\"ceph://um-mon01.osris.org\": {\"clustername\": \"osiris\","
                                                                                "\"config\": \"/etc/ceph/osiris.conf\","
                                                                                "\"pool\": \"dlt\","
                                                                                "\"crush_map\": \"None\"}," 
                                                "\"ibp://ibp2.crest.iu.edu:6714\": {\"duration\": 2592000}}"));

    PyRun_String(
		  "def main():\n"
		  "    bs = '5m'\n"
		  "    depots = None\n"
		  
		  "    if DEPOTS:\n"
		  "        try:\n"
		  "            depots = json.loads(DEPOTS)\n"
		  "        except Exception as e:\n"
		  "            print (\"ERROR: Could not read depot file: {}\".format(e))\n"
		  "            exit(1)\n"

		  "    host = UNIS_URL\n"
		  "    sess = libdlt.Session(host, bs=bs, depots=depots, **{\"viz_url\": None})\n"
		  "    sess.use_buffer = True\n"
		  "    sess.data_buffer = data\n"
                  "    sess.data_buffer_size = data_size\n"
		  "    xfer = sess.upload\n"
        
		  "    files = [filename]\n"
		  "    for f in files:\n"
		  "        diff, res = xfer(f, output)\n"
		  "        print (\"{0} ({1} {2:.2f} MB/s) {3}\".format(res.name, res.size, res.size/1e6/diff, res.selfRef))\n"

		  "    return res.selfRef\n"

 
		  "if __name__ == \"__main__\":\n"
		  "    selfRef = main()\n",
	 	  Py_file_input, globalDictionary, localDictionary);

    PyObject* pyStr = PyUnicode_AsEncodedString(PyDict_GetItemString(localDictionary, "selfRef"), "utf-8", "Error ~");
    char *selfRef = PyBytes_AsString(pyStr);

    Py_Finalize();

    return selfRef;
}
