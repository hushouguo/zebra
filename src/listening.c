/*
 * \file: listening.c
 * \brief: Created by hushouguo at Thu 16 Oct 2014 06:41:18 PM CST
 */

#include "zebra.h"

int listening_create(const char* ip, int port)
{
	int on;
	struct sockaddr_in sockaddr;
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
	{
		error_log("create socket failure:%d,%s\n", errno, errstring(errno));
		return -1;
	}

	on = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void*)&on, sizeof(on)) < 0)
	{
		error_log("setsockopt REUSEADDR error:%d,%s\n", errno, errstring(errno));
		return -1;
	}

	if (!setblocking(fd, false))
	{
		return -1;
	}
	
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = inet_addr(ip);
	sockaddr.sin_port = htons(port);

	if (bind(fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0)
	{
		error_log("bind address %s(%d) error:%d,%s\n", ip, port, errno, errstring(errno));
		return -1;
	}

	if (listen(fd, SOMAXCONN) < 0)
	{
		error_log("listen on %s(%d) error:%d,%s\n", ip, port, errno, errstring(errno));
		return -1;
	}

	/*
	if (!file_event_create(shard.el, shard.fd, EVENT_MASK_READABLE, accept_connection_handler, NULL))
	{
		panic("Unrecoverable error creating server.ipfd file event.\n");
	}
	*/

	debug_log("listening on %s:%d\n", ip, port);

	return fd;
}
