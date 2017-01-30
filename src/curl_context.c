// =============================================================================
//  periscope-ps (libunis-c)
//
//  Copyright (c) 2015-2016, Trustees of Indiana University,
//  All rights reserved.
//
//  This software may be modified and distributed under the terms of the BSD
//  license.  See the COPYING file for details.
//
//  This software was created at the Indiana University Center for Research in
//  Extreme Scale Technologies (CREST).
// =============================================================================
#include "curl_context.h"
#include "libunis_c_log.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


static char *__curl_do_operation(curl_context *cc, struct curl_slist *headers, int curl_opt, char *url,
                                 char *fields, char *data, curl_response **ret_data);
static void __init_curl_ssl(CURL *cc, long flags);
static size_t read_cb(void *ptr, size_t size, size_t nmemb, void *userp);
static size_t write_cb(void *ptr, size_t size, size_t nmemb, void *userp);
static void __set_curl_ssl_certificates(CURL *curl, char *capath, char *client_cert_path,
                                        char *client_cert_pass, char *client_keyname);

int init_curl(curl_context *cc, long flags) {
  if (cc->use_ssl) {
    if (CRYPTO_thread_setup()) {
      dbg_info(ERROR, "Couldn't setup SSL threads\n");
      return -1;
    }
  }

  if (cc->curl_persist) {
    cc->curl = curl_easy_init();
    if (!cc->curl) {
      dbg_info(ERROR, "Could not intialize CURL\n");
      return -1;
    }
    if (cc->use_ssl) {
      __init_curl_ssl(cc->curl, 0);
      __set_curl_ssl_certificates(cc->curl, cc->cacerts, cc->certfile, cc->keypass, cc->keyfile);
    }
  }

  if (cc->use_cookies) {
    if (!cc->cookiejar) {
      cc->cookiejar = tmpnam(NULL);
    }
  }

  return 0;
}

/*curl_cleanup :Call curl_easy_cleanup in case of persistant connection
 *
 */
int curl_cleanup(curl_context *cc) {

  if(cc == NULL) {
    return -1;
  }

  if (cc->curl_persist) {
    curl_easy_cleanup(cc->curl);
  }

  return 1;
}

static void __init_curl_ssl(CURL *curl, long flags) {
  // skip server verification
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
}

static void __set_curl_ssl_certificates(CURL *curl, char *server_ca_cert_path, char *client_cert_path,
                                        char *client_cert_pass, char *client_keyname) {
  // you can set this to anything because we have turned off verifypeer
  if (server_ca_cert_path != NULL)
    curl_easy_setopt(curl, CURLOPT_CAINFO, server_ca_cert_path);
  if (client_cert_path != NULL)
    curl_easy_setopt(curl,CURLOPT_SSLCERT, client_cert_path);
  if (client_cert_pass != NULL)
    curl_easy_setopt(curl,CURLOPT_KEYPASSWD, client_cert_pass);
  if (client_keyname != NULL)
    curl_easy_setopt(curl,CURLOPT_SSLKEY, client_keyname);

  curl_easy_setopt(curl,CURLOPT_SSLKEYTYPE, "PEM");
  curl_easy_setopt(curl,CURLOPT_SSLCERTTYPE,"PEM");

}

int copy_curl_context(curl_context *src, curl_context *dst) {
  memcpy(dst, src, sizeof (curl_context));
  return 0;
}

curl_response *init_curl_response() {
  curl_response *cr = malloc(sizeof(curl_response));
  cr->redirect_url = NULL;
  cr->content_type = NULL;
  cr->data = NULL;
  if (!cr)
    return NULL;
  memset(cr, 0, sizeof(curl_response));
  return cr;
}

int free_curl_response(curl_response *cr) {
  if (!cr)
    return -1;
  if (cr->redirect_url != NULL)
    free(cr->redirect_url);
  if (cr->content_type != NULL)
    free(cr->content_type);
  if (cr->data != NULL)
    free(cr->data);
  free(cr);
  return 0;
}

char *curl_post(curl_context *cc, char *url, char *accept_type, char *content_type,
                char *post_fields, char *post_data, curl_response **ret_data) {
  struct curl_slist *headers = NULL;
  if (accept_type) {
    headers = curl_slist_append(headers, accept_type);
  }
  if (content_type) {
    headers = curl_slist_append(headers, content_type);
  }
  return __curl_do_operation(cc, headers, CURLOPT_HTTPPOST, url, post_fields, post_data, ret_data);
}

char *curl_get(curl_context *cc, char *url, char *accept_type, curl_response **ret_data) {
  struct curl_slist *headers = NULL;
  if (accept_type) {
    headers = curl_slist_append(headers, accept_type);
  }
  return __curl_do_operation(cc, headers, CURLOPT_HTTPGET, url, NULL, NULL, ret_data);
}

char *curl_post_json_string(curl_context *cc, char *url, char *send_str, curl_response **ret_data) {
  struct curl_slist *headers = NULL;
  //headers = curl_slist_append(headers, "Transfer-Encoding: chunked");
  headers = curl_slist_append(headers, "Content-type: application/json,application/perfsonar+json");
  headers = curl_slist_append(headers, "Accept: text/html,application/json,application/perfsonar+json");
  return __curl_do_operation(cc, headers, CURLOPT_HTTPPOST, url, NULL, send_str, ret_data);
}

char *curl_get_json_string(curl_context *cc, char *url, curl_response **ret_data) {
  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "Accept: application/json,application/perfsonar+json");
  return __curl_do_operation(cc, headers, CURLOPT_HTTPGET, url, NULL, NULL, ret_data);
}

static char *__curl_do_operation(curl_context *cc, struct curl_slist* headers, int curl_opt,
                                 char *url, char *fields, char *data, curl_response **ret_data) {

  CURL *curl = NULL;
  CURLcode res;
  char *endpoint;
  char *tmpptr;
  long tmplong;
  long send_len;

  *ret_data = init_curl_response();
  if (!(*ret_data)) {
    goto error_exit;
  }

  if (data) {
    send_len = strlen(data);

  }
  else {
    send_len = 0;
  }

  if (url) {
    endpoint = url;
  }
  else {
    endpoint = cc->url;
  }

  struct curl_http_data send_data = {
    .ptr = data,
    .len = send_len
  };

  struct curl_http_data recv_data = {
    .ptr = NULL,
    .len = 0
  };

  if (cc->curl_persist) {
    curl = cc->curl;
  }
  else {
    curl = curl_easy_init();
    if (!curl) {
      dbg_info(ERROR, "Could not initialize CURL\n");
      goto error_exit;
    }
    if (cc->use_ssl) {
      __init_curl_ssl(curl, 0);
      __set_curl_ssl_certificates(curl, cc->cacerts, cc->certfile, cc->keypass, cc->keyfile);
    }
  }

  //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

  curl_easy_setopt(curl, CURLOPT_URL, endpoint);
  curl_easy_setopt(curl, curl_opt, 1L);
  curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_cb);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
  curl_easy_setopt(curl, CURLOPT_READDATA, &send_data);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &recv_data);

  if (cc->follow_redirect) {
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  }

  if (cc->use_cookies) {
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cc->cookiejar);
    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cc->cookiejar);
  }

  if (headers) {
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  }

  if (fields && (curl_opt & CURLOPT_HTTPPOST)) {
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, fields);
  }
  else if (curl_opt & CURLOPT_HTTPPOST) {
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, send_len);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, NULL);
  }

  res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    dbg_info(ERROR, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    goto error_exit;
  }

  if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &tmplong) == CURLE_OK)
    (*ret_data)->status = tmplong;

  if (curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL, &tmpptr) == CURLE_OK) {
    if(tmpptr != NULL) {
      asprintf(&((*ret_data)->redirect_url), "%s", tmpptr);
    }
  }

  if (curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &tmpptr) == CURLE_OK) {
    if(tmpptr != NULL) {
      (*ret_data)->content_type = malloc(strlen(tmpptr) + 1);
      strncpy((*ret_data)->content_type, tmpptr, strlen(tmpptr) + 1);
      //asprintf(&((*ret_data)->content_type), "%s", tmpptr);
    }
  }

  if (recv_data.len) {
    (*ret_data)->data = malloc(recv_data.len+1 * sizeof(char));
    memcpy((*ret_data)->data, recv_data.ptr, recv_data.len);
    (*ret_data)->data[recv_data.len] = '\0';
  }

  if (headers) {
    curl_slist_free_all(headers);
  }

  if (!(cc->curl_persist)) {
    curl_easy_cleanup(curl);
  }

  return (*ret_data)->data;

error_exit:
  if (headers)
    curl_slist_free_all(headers);
  if (!(cc->curl_persist))
    curl_easy_cleanup(curl);
  free_curl_response(*ret_data);
  *ret_data = NULL;
  return NULL;
}

static size_t read_cb(void *ptr, size_t size, size_t nmemb, void *userp) {
  struct curl_http_data *data = (struct curl_http_data *) userp;

  if (size * nmemb < 1) {
    return 0;
  }
  if (data->len) {
    *(char *) ptr = data->ptr[0];
    data->ptr++;
    data->len--;
    return 1;
  }

  return 0;
}

static size_t write_cb(void *ptr, size_t size, size_t nmemb, void *userp) {
  struct curl_http_data *s = (struct curl_http_data *) userp;
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}
