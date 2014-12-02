/*
 * \file: worker_handler.h
 * \brief: Created by hushouguo at Oct 29 2014 23:55:40
 */
 
#ifndef __WORKER_HANDLER_H__
#define __WORKER_HANDLER_H__

typedef struct worker_handler
{
	int						id;
	msg_queue_t*			q;
	lua_stack_t*			stack;
	lua_pb_t*				pb;
	hash_table_t*			http_table;
	hash_table_t*			client_table;
	hash_table_t*			database_table;
} worker_handler_t;

worker_handler_t* handler_create(int n);
void handler_delete(worker_handler_t* handler);

worker_handler_t* find_handler_by_stack(lua_State* L);
worker_handler_t* find_handler_by_fd(int fd);
worker_handler_t* find_handler_by_userid(uint64_t userid);

/* cross thread post message */
void handler_post_message(connection_t* c, nullmsg_t* msg);

#endif
