/*
 * \file: connection.c
 * \brief: Created by hushouguo at Sat 25 Oct 2014 08:38:06 AM CST
 */

#include "zebra.h"

#define TCPWINDOW	512*KB

connection_t* connection_create(int fd, struct client* client)
{
	size_t size = TCPWINDOW;
	connection_t* c = NULL;

	if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (const void*)&size, sizeof(size)) < 0)
	{
		error_log("setsockopt SO_RCVBUF error:%d,%s\n", errno, errstring(errno));
		close(fd);
		return NULL;
	}

	if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (const void*)&size, sizeof(size)) < 0)
	{
		error_log("setsockopt SO_SNDBUF error:%d,%s\n", errno, errstring(errno));
		close(fd);
		return NULL;
	}

	c = (connection_t*)malloc(sizeof(connection_t));
	memset(c, 0, sizeof(connection_t));
	c->fd = fd;
	c->rbb = byte_buffer_create(DEF_BYTE_BUFFER_SIZE);
	c->wrbb = byte_buffer_create(DEF_BYTE_BUFFER_SIZE);
	c->wwbb = byte_buffer_create(DEF_BYTE_BUFFER_SIZE);
	c->wlocker = 0;
	c->need_close = false;
	c->client = client;
	c->last_recv_time = time(0);

	setblocking(fd, false);

	return c;
}

void connection_delete(connection_t* c)
{
	if (c)
	{
		close(c->fd);
		byte_buffer_delete(c->rbb);
		byte_buffer_delete(c->wrbb);
		byte_buffer_delete(c->wwbb);
		SAFE_FREE(c);
	}
}

