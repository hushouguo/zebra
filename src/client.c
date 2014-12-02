/*
 * \file: client.c
 * \brief: Created by hushouguo at Nov 07 2014 10:33:44
 */

#include "zebra.h"

client_t* client_create(struct worker_handler* handler)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	client_t* client;
	if (fd < 0)
	{
		error_log("create socket error:%d,%s\n", errno, errstring(errno));
		return NULL;
	}
	client = (client_t*)malloc(sizeof(client_t));
	memset(client, 0, sizeof(client_t));
	client->c = connection_create(fd, client);
	if (!client->c)
	{
		client_delete(client);
		return NULL;
	}
	client->owner = handler;
	return client;
}

void client_delete(client_t* client)
{
	if (client)
	{
		if (client->c)
		{
			connection_delete(client->c);
		}
		SAFE_FREE(client);
	}
}

bool client_connect(client_t* client, const char* ip, int port, const char* domain)
{
	int fd;
	struct sockaddr_in serveraddr;

	if (!client->c)
	{
		/* connection shutdown, recreate socket */
		fd = socket(AF_INET, SOCK_STREAM, 0);
		client->c = connection_create(fd, client);
	}
	else
	{
		fd = client->c->fd;
	}

	/* Todo: domain lookup */

	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(ip);
	serveraddr.sin_port = htons(port);

	setblocking(fd, false);
	if (connect(fd, (struct sockaddr*)&serveraddr, sizeof(struct sockaddr_in)) < 0)
	{
		fd_set fdread;
		fd_set fdwrite;
		struct timeval tv;
	
		FD_ZERO(&fdread);
		FD_ZERO(&fdwrite);
		FD_SET(fd, &fdread);
		FD_SET(fd, &fdwrite);
	
		tv.tv_sec = DEF_CLIENT_CONNECT_EXPIRE;
		tv.tv_usec = 0;
	
		int retval = select(fd+1, &fdread, &fdwrite, NULL, &tv);
		if (retval < 0)
		{
			error_log("select operate failure:%d,%s\n", errno, errstring(errno));
			close(fd);
			return false;
		}
		else if (retval == 0) 
		{
			/*error_log("connect timeout, unreachable server %s:%d\n", inet_ntoa(*(struct in_addr*)&ip), ntohs(port));*/
			error_log("connect timeout, unreachable server %s:%d\n", ip, port);
			close(fd);
			return false;
		}
		else
		{
			if (FD_ISSET(fd, &fdwrite))
			{
				if (FD_ISSET(fd, &fdread))
				{
					//Note: NOT SURE, as a failure case
					error_log("connect exception, unreachable server %s:%d\n", ip, port);
					close(fd);
					return false;
				}
				else
				{
					/* debug_log("connect success\n"); */
				}
			}
			else
			{
				error_log("connect failure, unreachable server %s:%d\n", ip, port);
				close(fd);
				return false;
			}
		}
	}

	debug_log("succeed connect to server:%s:%d\n", ip, port);

	strncpy(client->ip, ip, sizeof(client->ip));
	client->port = port;
	if (domain) { strncpy(client->domain, domain, sizeof(client->domain)); }

	return true;
}

bool client_reconnect(client_t* client)
{
	if (client->c)
	{
		connection_delete(client->c);
		client->c = NULL;
	}
	return client_connect(client, client->ip, client->port, client->domain);
}

