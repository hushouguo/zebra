/*
 * \file: http.h
 * \brief: Created by hushouguo at Nov 10 2014 08:44:57
 */
 
#ifndef __HTTP_H__
#define __HTTP_H__

#define MAX_HTTP_URL_LENGTH			1024
typedef struct http_request
{
	uint32_t					id;
	struct evbuffer*			evbuffer;
	struct evhttp_request*		evrequest;
	uint32_t					bufsize;
} http_request_t;

http_request_t* http_request_create(struct evhttp_request* evreq);
void http_request_delete(http_request_t* req);
void http_request_print_text(http_request_t* req, const char* text);
void http_request_print_data(http_request_t* req, void* b, size_t size);
const char*	http_request_get_input_header(http_request_t* req, const char* key);
void http_request_add_header(http_request_t* req, const char* key, const char* value);
void http_request_remove_header(http_request_t* req, const char* key);
void http_request_clear_headers(http_request_t* req);
const char*	http_request_remote_host(http_request_t* req);
int	http_request_remote_port(http_request_t* req);
const char*	http_request_get_read_buffer(http_request_t* req);
void http_request_complete_request(http_request_t* req);

typedef void (*http_callback)(int, int, const char*, void*);
typedef struct http
{
	uint32_t					id;
	char						url[MAX_HTTP_URL_LENGTH];
	struct event_base*			evbase;
	struct evhttp*				evhttp;
	hash_table_t*				request_table;
	http_callback				cb;
	void*						cb_data;
} http_t;

http_t* http_create(const char* ip, int port, http_callback cb, void* cb_data);
void http_delete(http_t* http);
void http_loop(http_t* http);
void http_finish_request(http_t* http, http_request_t* req);
http_request_t* http_find_request(http_t* http, uint32_t id);

void lua_stack_register_http_functions(lua_stack_t* stack);

#endif
