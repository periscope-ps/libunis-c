/****************************
Libunis-c Exnode Interface  *
*****************************/

#include "exnode_data.h"

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

    if(result->pos + size * nmemb >= BUFFER_SIZE - 1)
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

    data = malloc(BUFFER_SIZE);
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
    if(data)
        free(data);
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
    //exnode *xnode_data = exnode_data;
    int index = 0;
    exnode *xnode;
    json_t *arr_data;
    json_t *root;
    json_error_t error;
    parent *parentt;
    json_t *p, *href, *rel;
    json_t *schema, *name, *selfRef, *created, *modified, *ts, *mode, *id, *size, *extents;
    //extent **extents;
    //struct exnode **exnodes;
    xnode_stack *xnode_st;

    xnode_st = (xnode_stack *) malloc (1 * sizeof(xnode_stack));
    memset(xnode_st, 0, sizeof(xnode_stack));

    //printf("Text is %s\n", text);
    root = json_loads(text, 0, &error);
    free(text);

    if(!root)
    {
        fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
        return 0;
    }

    if(!json_is_array(root))
    {
	arr_data = json_array();
	json_array_append_new(arr_data, root);
	root = arr_data;
        //printf("------> root is not an array\n");
        //json_decref(root);
        //return 1;
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
            return 0;
        }

        p = json_object_get(data, "parent");
        if(!json_is_object(p))
        {
	    if (json_is_null(p))
	    {
		//fprintf(stderr, "error: parent is null\n");
	    	parentt = NULL;
		xnode->parent = parentt;
	    }
	    else
	    {
            	fprintf(stderr, "error: [%d] parent is not a json object\n", (int)(i + 1));
            	return 0;
	    }
	    //parentt = NULL;
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

	//udata[index].href = json_string_value(href);

        schema = json_object_get(data, "$schema");
        if(!json_is_string(schema))
        {
            fprintf(stderr, "error: [%d] schema is not a string\n", (int)(i + 1));
            json_decref(root);
            return 0;
        }
	xnode->schema = (char*) malloc ((strlen(json_string_value(schema))+1) * sizeof(char));
	memset(xnode->schema, 0, strlen(xnode->schema));
	strcpy(xnode->schema, json_string_value(schema));

        name = json_object_get(data, "name");
        if(!json_is_string(name))
        {
            fprintf(stderr, "error: [%d] name is not a string\n", (int)(i + 1));
            json_decref(root);
            return 0;
        }
	//xnode->name = json_string_value(name);
	xnode->name = (char*) malloc ((strlen(json_string_value(name))+1) * sizeof(char));
	memset(xnode->name, 0, strlen(xnode->name));
	strcpy(xnode->name, json_string_value(name));

        selfRef = json_object_get(data, "selfRef");
        if(!json_is_string(selfRef))
        {
            fprintf(stderr, "error: [%d] selfRef is not a string\n", (int)(i + 1));
            json_decref(root);
            return 0;
        }
	//xnode->selfRef = json_string_value(selfRef);
	xnode->selfRef = (char*) malloc ((strlen(json_string_value(selfRef))+1) * sizeof(char));
	memset(xnode->selfRef, 0, strlen(xnode->selfRef));
	strcpy(xnode->selfRef, json_string_value(selfRef));

        mode = json_object_get(data, "mode");
        if(!json_is_string(mode))
        {
            fprintf(stderr, "error: [%d] mode is not a string\n", (int)(i + 1));
            json_decref(root);
            return 0;
        }
	//xnode->mode = json_string_value(mode);
	xnode->mode = (char*) malloc ((strlen(json_string_value(mode))+1) * sizeof(char));
	memset(xnode->mode, 0, strlen(xnode->mode));
	strcpy(xnode->mode, json_string_value(mode));

        id = json_object_get(data, "id");
        if(!json_is_string(id))
        {
            fprintf(stderr, "error: [%d] id is not a string\n", (int)(i + 1));
            json_decref(root);
            return 0;
        }
	//xnode->id = json_string_value(id);
	xnode->id = (char*) malloc ((strlen(json_string_value(id))+1) * sizeof(char));
	memset(xnode->id, 0, strlen(xnode->id));
	strcpy(xnode->id, json_string_value(id));

        size = json_object_get(data, "size");
        if(!json_is_number(size))
        {
            fprintf(stderr, "error: [%d] size is not an integer\n", (int)(i + 1));
            json_decref(root);
            return 0;
        }
	xnode->size = json_integer_value(size);

	xnode->child_cnt = 0;

	//printf("parent = %s\nname = %s\nselfRef = %s\nmode = %s\nid = %s\nsize = %ld\nschema = %s\n-------\n\n", 
	//	xnode->parent->href, xnode->name, xnode->selfRef, xnode->mode, xnode->id, xnode->size, xnode->schema);
	xnode_st->exnode_data[index] = xnode;
	//printf("exnode name = %s\n", xnode_st->exnode_data[index]->name);
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
    free(xs->exnode_data[i]);
  }

  free(xs);

  return 0;
}

xnode_stack * retrieve_exnodes(
				char url[]
			      )
{
    char *text;
    //char url[URL_SIZE];
    //exnode *exnode_data[2000] = {0};
    int xnode_cnt = 0;
    xnode_stack *xnode_st;

    //sprintf(url, UNIS_URL);
    //printf ("\nURL is %s\n", url);

    text = request(url);

    if (!text)
    {
        return NULL;
    }

    xnode_st = process_exnode(text);

    //free_exnodes(exnode_data, xnode_cnt);

    return xnode_st;
} 

void insert_exnode(
		    exnode *parent_xnode, 
		    exnode *child_exnode
		  )
{
    if (NULL != parent_xnode)
    {
       parent_xnode->child[parent_xnode->child_cnt] = (struct exnode *) malloc (1 * sizeof(exnode));
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
      //printf( " %s\n", token );
    
      token = strtok(NULL, s);
      if (NULL != token)
      {
	 token2 = token;
      }
   }
   
   //printf("parent id is %s\n", token2);

   return token2;
}

exnode * find_parent (
		        exnode *xnode,
		        char *parent_id
	 	     )
{
  	int i = 0;
	exnode *xnod = NULL;

	//printf("path is %s\n", xnode->name);

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

       if (xnode_s->exnode_data[i]->parent != NULL)
       {
          parent_id = get_parent_id(xnode_s->exnode_data[i]->parent->href);
          parent_found = 0;
	  //printf("----> parent id -> %s\n", parent_id);


	  if (NULL != parent_id)
	  {
             for (j = 0; j < index; j++)
             {
	        xnod = find_parent(xnode_parent->exnode_data[j], parent_id);

	        if (NULL != xnod)
	        {
		   //printf("---------> xnode id -> %s\n", xnod->id);
		   //parent_found = 1;
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
		   //parent_found = 1;
		   break;
	        }
	     }
          }
	     
	  if (NULL != xnod)  //(strncmp(parent_id, xnode_parent->exnode_data[j]->id, strlen(parent_id)) == 0)
	  {
	     insert_exnode(xnod, xnode_s->exnode_data[i]);
	     parent_found = 1;
	  }
          else   //if (parent_found == 0)
          {
	     xnode_stack *xs = retrieve_exnodes(xnode_s->exnode_data[i]->parent->href);
	     xnode_parent->exnode_data[index] = xs->exnode_data[0];
	     insert_exnode(xnode_parent->exnode_data[index], xnode_s->exnode_data[i]);
	     index++;
          }
       }
       else
       {
	  xnode_parent->exnode_data[index] = xnode_s->exnode_data[i];
          index++;
       }
    }
    xnode_parent->exnode_cnt = index;

    return xnode_parent;
}

