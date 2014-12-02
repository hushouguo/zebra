/*
 * \file: connection.h
 * \brief: Created by hushouguo at Thu 16 Oct 2014 10:23:34 PM CST
 */
 
#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#define DEF_BYTE_BUFFER_SIZE	256

struct client;
typedef struct connection
{
	int					fd;
	byte_buffer_t*		rbb;
	byte_buffer_t*		wrbb;
	byte_buffer_t*		wwbb;
	atomic_t			wlocker;
	bool				need_close;
	struct client*		client;
	time_t				last_recv_time;
} connection_t;

connection_t* connection_create(int fd, struct client* client);
void connection_delete(connection_t* c);

#endif
