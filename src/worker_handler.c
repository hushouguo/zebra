/*
 * \file: worker_handler.c
 * \brief: Created by hushouguo at Oct 29 2014 23:58:10
 */

#include "zebra.h"

worker_handler_t* find_handler_by_stack(lua_State* L)
{
	int n;
	for (n = 0; n < shard->pool_size; ++n)
	{
		worker_handler_t* handler = shard->worker_handler_pool[n];
		if (handler->stack->L == L) { return handler; }
	}
	return NULL;
}

worker_handler_t* find_handler_by_fd(int fd)
{
	return shard->worker_handler_pool[fd%shard->pool_size];
}

worker_handler_t* find_handler_by_userid(uint64_t userid)
{
	return shard->worker_handler_pool[userid%shard->pool_size];
}

worker_handler_t* handler_create(int n)
{
	worker_handler_t* handler = (worker_handler_t*)malloc(sizeof(worker_handler_t));
	memset(handler, 0, sizeof(worker_handler_t));
	handler->id = n;
	handler->q = msg_queue_create();
	handler->stack = lua_stack_create(LUA_STACK_SIZE);
	handler->pb = lua_pb_create("protocol/protocol.xml");
	if (!handler->pb)
	{
		handler_delete(handler);
		return NULL;
	}
	handler->http_table = hash_table_create(16);
	handler->client_table = hash_table_create(8);
	handler->database_table = hash_table_create(8);

	/*
	 * register `cc` namespace
	 */
	lua_stack_register_namespace(handler->stack, LUA_REGISTER_NAMESPACE);

	/*
	 * register standard functions
	 */
	lua_stack_register_standard_functions(handler->stack);

	/*
	 * register logger functions
	 */
	lua_stack_register_logger_functions(handler->stack);

	/*
	 * register database functions
	 */
	lua_stack_register_record_functions(handler->stack);

	/*
	 * register networking functions
	 */
	lua_stack_register_network_functions(handler->stack);

	/*
	 * register http functions
	 */
	lua_stack_register_http_functions(handler->stack);

	/*
	 * register lua cURL
	 */
	luaopen_cURL(handler->stack->L);

	/* run main script file */
	if (!lua_stack_execute_script_file(handler->stack, shard->main_script_file))
	{
		handler_delete(handler);
		return NULL;
	}
	return handler;
}
void handler_delete(worker_handler_t* handler)
{
	if (handler)
	{
		msg_queue_delete(handler->q);
		lua_stack_delete(handler->stack);
		lua_pb_delete(handler->pb);
		hash_table_delete(handler->client_table);
		hash_table_delete(handler->http_table);
		hash_table_delete(handler->database_table);
		SAFE_FREE(handler);
	}
}
void handler_post_message(connection_t* c, nullmsg_t* msg)
{
	if (msg)
	{
		worker_handler_t* handler = find_handler_by_userid(msg->userid);
		msg_queue_push(handler->q, c->fd, msg);
	}
	else
	{
		int n;
		for (n = 0; n < shard->pool_size; ++n)
		{
			worker_handler_t* handler = shard->worker_handler_pool[n];
			msg_queue_push(handler->q, c->fd, msg);
		}
	}
}


