/*
 * \file: http.c
 * \brief: Created by hushouguo at Nov 10 2014 10:16:30
 */

#include "zebra.h"

static uint32_t http_request_baseid = 1000;
http_request_t* http_request_create(struct evhttp_request* evreq)
{
	http_request_t* req = (http_request_t*)malloc(sizeof(http_request_t));
	memset(req, 0, sizeof(http_request_t));
	req->id = http_request_baseid++;
	req->evbuffer = evbuffer_new();
	req->evrequest = evreq;
	req->bufsize = 0;
	return req;
}

void http_request_delete(http_request_t* req)
{
	if (req)
	{
		if (req->evbuffer)
		{
			evbuffer_free(req->evbuffer);
			req->evbuffer = NULL;
		}
		SAFE_FREE(req);
	}
}

void http_request_print_text(http_request_t* req, const char* text)
{
	size_t len;
	if (!req->evrequest)
	{
		error_log("Invalid evrequest\n");
		return;
	}
	if (!text) { text = "NULL"; }
	len = strlen(text);
	evbuffer_add(req->evbuffer, text, len);
	req->bufsize += len;
}

void http_request_print_data(http_request_t* req, void* b, size_t size)
{
	if (!req->evrequest)
	{
		error_log("Invalid evrequest\n");
		return;
	}
	if (!b) { b = "NULL", size = 4; }
	evbuffer_add(req->evbuffer, b, size);
	req->bufsize += size;
}

const char*	http_request_get_input_header(http_request_t* req, const char* key)
{
	if (!req->evrequest)
	{
		error_log("Invalid evrequest\n");
		return "";
	}
	return evhttp_find_header(req->evrequest->input_headers, key);
}

void http_request_add_header(http_request_t* req, const char* key, const char* value)
{
	if (!req->evrequest)
	{
		error_log("Invalid evrequest\n");
		return;
	}
	evhttp_add_header(req->evrequest->output_headers, key, value);
}

void http_request_remove_header(http_request_t* req, const char* key)
{
	if (!req->evrequest)
	{
		error_log("Invalid evrequest\n");
		return;
	}
	evhttp_remove_header(req->evrequest->output_headers, key);
}

void http_request_clear_headers(http_request_t* req)
{
	if (!req->evrequest)
	{
		error_log("Invalid evrequest\n");
		return;
	}
	evhttp_clear_headers(req->evrequest->output_headers);
}

const char*	http_request_remote_host(http_request_t* req)
{
	if (!req->evrequest)
	{
		error_log("Invalid evrequest\n");
		return "";
	}
	if (!req->evrequest->remote_host)
	{
		error_log("not found evrequest->remote_host\n");
		return "";
	}
	return req->evrequest->remote_host;
}

int	http_request_remote_port(http_request_t* req)
{
	if (!req->evrequest)
	{
		error_log("Invalid evrequest\n");
		return 0;
	}
	return req->evrequest->remote_port;
}

const char*	http_request_get_read_buffer(http_request_t* req)
{
	size_t size;
	char* buf;
	if (!req->evrequest)
	{
		error_log("Invalid evrequest\n");
		return "";
	}

	size = EVBUFFER_LENGTH(req->evrequest->input_buffer);
	evbuffer_expand(req->evrequest->input_buffer, 1);

	buf = (char*)EVBUFFER_DATA(req->evrequest->input_buffer);
	buf[size] = 0;

	return buf;
}

void http_request_complete_request(http_request_t* req)
{
	if (!req->evrequest)
	{
		error_log("Invalid evrequest\n");
		return;
	}
	evhttp_send_reply(req->evrequest, HTTP_OK, "OK", req->evbuffer);
	req->evrequest = NULL;
}

/***********************************************************************************************************************/

static void do_request_get(http_t* http, http_request_t* request, struct evhttp_request* req)
{
	const char* uri = evhttp_request_get_uri(req);
	char* decode_uri = evhttp_decode_uri(uri);
	debug_log("GET [0x%x]: %s\n", request->id, decode_uri);
	/*debug_log("uri:%s\n", uri);*/

	http->cb(http->id, request->id, decode_uri, http->cb_data);
	
	free(decode_uri);
}

static void do_request_post(http_t* http, http_request_t* request, struct evhttp_request* req)
{
	char* decode_uri;
	struct evbuffer* buf = evhttp_request_get_input_buffer(req);
	char buffer[4096];
	snprintf(buffer, sizeof(buffer), "%s?", evhttp_request_get_uri(req));
	while (evbuffer_get_length(buf)) 
	{
		char cbuf[128];
		int n = evbuffer_remove(buf, cbuf, sizeof(buf)-1);
		if (n > 0)
		{
			cbuf[n] = '\0';
			strncat(buffer, cbuf, sizeof(buffer) - strlen(buffer) - 1);
		}
	}

	/*debug_log("POST uri:%s\n", buffer);*/

	decode_uri = evhttp_decode_uri(buffer);

	debug_log("POST [0x%x]: %s\n", request->id, decode_uri);
	
	http->cb(http->id, request->id, decode_uri, http->cb_data);

	free(decode_uri);
}

static void do_request(struct evhttp_request* req, void* p)
{
	http_t* http = (http_t*)p;
	http_request_t* request = http_request_create(req);
	hash_table_insert(http->request_table, request->id, request);
	/*
	{
		struct evkeyvalq* headers = evhttp_request_get_input_headers(req);
		struct evkeyval* header;
		debug_log("Headers:\n");
		for (header = headers->tqh_first; header; header = header->next.tqe_next) 
		{
			debug_log("  %s: %s\n", header->key, header->value);
		}
	}
	*/
	switch (evhttp_request_get_command(req))
	{
		case EVHTTP_REQ_GET: do_request_get(http, request, req); break;
		case EVHTTP_REQ_POST: do_request_post(http, request, req); break;
		default: error_log("unhandled http req:%d\n", evhttp_request_get_command(req));
	}
}

static uint32_t http_baseid = 1;
http_t* create_http(const char* ip, int port, http_callback cb, void* cb_data)
{
	http_t* http = (http_t*)malloc(sizeof(http_t));
	memset(http, 0, sizeof(http_t));
	http->id = http_baseid++;
	http->evbase = event_base_new();
	http->evhttp = evhttp_new(http->evbase);
	http->request_table = hash_table_create(128);
	if (!http->evbase || !http->evhttp)
	{
		error_log("evbase:%p, evhttp:%p\n", http->evbase, http->evhttp);
		http_delete(http);
		return NULL;
	}
	if (evhttp_bind_socket(http->evhttp, ip, port) < 0)
	{
		error_log("evhttp_bind_socket failure:%s:%d\n", ip, port);
		http_delete(http);
		return NULL;
	}
	evhttp_set_gencb(http->evhttp, do_request, http);
	http->cb = cb;
	http->cb_data = cb_data;
	snprintf(http->url, sizeof(http->url), "http://%s:%d/", ip, port);
	debug_log("http listening on %s:%d, url:%s\n", ip, port, http->url);
	return http;	
}

void http_delete(http_t* http)
{
	if (http)
	{
		if (http->evhttp) { evhttp_free(http->evhttp); }
		if (http->request_table) { hash_table_delete(http->request_table); }
		SAFE_FREE(http);
	}
}

void http_loop(http_t* http)
{
	int retval = event_base_loop(http->evbase, EVLOOP_ONCE | EVLOOP_NONBLOCK);
	if (retval != 0)
	{
		error_log("event_base_loop error:%d\n", retval);
		return;
	}
}

void finish_http_request(http_t* http, http_request_t* req)
{
	http_request_complete_request(req);
	hash_table_remove(http->request_table, req->id);
	SAFE_FREE(req);
}

http_request_t* http_find_request(http_t* http, uint32_t id)
{
	return (http_request_t*)hash_table_find(http->request_table, id);	
}

/* ------------------------------------------------------------------------------------------------------------ */

static void do_http_request(int httpid, int reqid, const char* uri, void* cb_data)
{
	worker_handler_t* handler = (worker_handler_t*)cb_data;
	lua_protocol_http_request(handler->stack, httpid, reqid, uri);
}

/* int create_http(ip, port) */
static int cc_create_http(lua_State* L)
{
	const char* ip;
	int port;
	http_t* http;
	worker_handler_t* handler = find_handler_by_stack(L);
	int args = lua_gettop(L);
	if (args < 2)
	{
		error_log("`%s` parameter lack:%d\n", __FUNCTION__, args);
		return 0;
	}

	if (!lua_isstring(L, -args))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -args)));
		return 0;
	}

	if (!lua_isnumber(L, -(args-1)))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -(args-1))));
		return 0;
	}

	ip = lua_tostring(L, -args);
	port = lua_tointeger(L, -(args-1));
	http = create_http(ip, port, do_http_request, handler);
	if (!http)
	{
		return 0;
	}
	hash_table_insert(handler->http_table, http->id, http);
	lua_pushinteger(L, http->id);
	return 1;
}
/* void print_http(httpid, reqid, text) */
static int cc_print_http(lua_State* L)
{
	uint32_t httpid;
	uint32_t reqid;
	const char* text;
	http_t* http;
	http_request_t* request;
	worker_handler_t* handler = find_handler_by_stack(L);
	int args = lua_gettop(L);
	if (args < 3)
	{
		error_log("`%s` parameter lack:%d\n", __FUNCTION__, args);
		return 0;
	}

	if (!lua_isnumber(L, -args))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -args)));
		return 0;
	}

	if (!lua_isnumber(L, -(args-1)))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -(args-1))));
		return 0;
	}

	if (!lua_isstring(L, -(args-2)))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -(args-2))));
		return 0;
	}

	httpid = lua_tointeger(L, -args);
	reqid = lua_tointeger(L, -(args-1));
	text = lua_tostring(L, -(args-2));

	http = (http_t*)hash_table_find(handler->http_table, httpid);
	if (!http)
	{
		error_log("`%s` not found httpid:%d\n", __FUNCTION__, httpid);
		return 0;
	}

	request = http_find_request(http, reqid);
	if (!request)
	{
		error_log("`%s` not found reqid:%d\n", __FUNCTION__, reqid);
		return 0;
	}

	http_request_print_text(request, text);

	return 0;
}
/* void finish_http(httpid, reqid) */
static int cc_finish_http(lua_State* L)
{
	uint32_t httpid;
	uint32_t reqid;
	http_t* http;
	http_request_t* request;
	worker_handler_t* handler = find_handler_by_stack(L);
	int args = lua_gettop(L);
	if (args < 2)
	{
		error_log("`%s` parameter lack:%d\n", __FUNCTION__, args);
		return 0;
	}

	if (!lua_isnumber(L, -args))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -args)));
		return 0;
	}

	if (!lua_isnumber(L, -(args-1)))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -(args-1))));
		return 0;
	}

	httpid = lua_tointeger(L, -args);
	reqid = lua_tointeger(L, -(args-1));

	http = (http_t*)hash_table_find(handler->http_table, httpid);
	if (!http)
	{
		error_log("`%s` not found httpid:%d\n", __FUNCTION__, httpid);
		return 0;
	}

	request = http_find_request(http, reqid);
	if (!request)
	{
		error_log("`%s` not found reqid:%d\n", __FUNCTION__, reqid);
		return 0;
	}

	finish_http_request(http, request);

	return 0;
}

void lua_stack_register_http_functions(lua_stack_t* stack)
{
	lua_stack_register_function(stack, "create_http", cc_create_http);		/* int create_http(ip, port) */ 
	lua_stack_register_function(stack, "print_http", cc_print_http);		/* void print_http(httpid, reqid, text); */
	lua_stack_register_function(stack, "finish_http", cc_finish_http);		/* void finish_http(httpid, reqid) */
}

