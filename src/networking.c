/*
 * \file: networking.c
 * \brief: Created by hushouguo at Thu 16 Oct 2014 10:55:11 AM CST
 */

#include "zebra.h"

bool setblocking(int fd, bool block)
{
	int opts = fcntl(fd, F_GETFL);
	if (opts < 0)
	{
		error_log("fcntl(%d):%d,%s\n", fd, errno, errstring(errno));
		return false;
	}
	if (block)
	{
		opts &= ~(O_NONBLOCK);
	}
	else
	{
		opts |= O_NONBLOCK;
	}
	if (fcntl(fd, F_SETFL, opts) < 0)
	{
		error_log("fcntl(%d):%d,%s\n", fd, errno, errstring(errno));
		return false;
	}
	return true;
}
bool interrupted()
{
#ifdef WIN32
	return WSAGetLastError() == WSAEINTR ? true : false;
#else
	return errno == EINTR ? true : false;
#endif	
}
bool wouldblock()
{
#ifdef WIN32
	int err = WSAGetLastError();
	return err == WSAEWOULDBLOCK || err == WSA_IO_PENDING || err == ERROR_IO_PENDING ? true : false;
#else
	return errno == EAGAIN || errno == EWOULDBLOCK ? true : false;
#endif	
}
bool connectionlost()
{
#ifdef WIN32
	int err = WSAGetLastError();
	return err == WSAECONNRESET || err == WSAESHUTDOWN || err == WSAENOTCONN || err == WSAECONNABORTED ? true : false;
#else
	return errno == ECONNRESET || errno == ENOTCONN || errno == ESHUTDOWN || errno == ECONNABORTED || errno == EPIPE ? true : false;
#endif	
}

/* -------------------------------------------------------------------------------------------------------------- */

void close_connection(int fd, bool passive)
{
	connection_t* c = shard->conntable[fd];
	/*debug_log("fd:%d close connection\n", fd);*/
	if (c)
	{
		if (c->client) { c->client->c = NULL; }
		connection_delete(c);
		file_event_delete(shard->el, fd, EVENT_MASK_ALL);
		shard->conntable[fd] = NULL;
		/* active&passive both notice lua */
		/*if (passive)*/
		{
			handler_post_message(c, (nullmsg_t*)NULL);
		}
	}
	else
	{
		error_log("not exist fd:%d or already close\n", fd);
	}
}

static void read_connection_handler(event_loop_t* el, int fd, void* data, int mask)
{
	connection_t* c = shard->conntable[fd];
	assert(c != NULL && c == data);
	if (c->need_close)
	{
		close_connection(fd, false);/* force close */
		return;
	}
	while (!shard->halt)
	{
		ssize_t retval;
		if (byte_buffer_remain(c->rbb) == 0) { byte_buffer_resize(c->rbb, DEF_BYTE_BUFFER_SIZE); }
		retval = recv(fd, byte_buffer_wb(c->rbb), byte_buffer_remain(c->rbb), MSG_DONTWAIT);
		if (retval == 0)
		{
			close_connection(fd, true);/* lost connection */
			return;
		}
		if (retval < 0)
		{
			if (interrupted()) { continue; }
			if (wouldblock())
			{
				break;
			}
			close_connection(fd, true);/* recv error */
			return;
		}

		c->rbb->current += (size_t)retval;

		while (1)
		{
			unsigned short msg_size;
			if (c->rbb->current < sizeof(unsigned short)) { break; }
			msg_size = *(unsigned short*)&c->rbb->buf[0];
			if (msg_size < sizeof(nullmsg_t))
			{
				error_log("illegue message len:%d\n", msg_size);
				close_connection(fd, true);/* illegue message */
				return;
			}
			if (c->rbb->len < msg_size)
			{
				if (msg_size > MAX_NETWORK_PACKAGE_SIZE)
				{
					error_log("exceed package size limit:%d\n", msg_size);
					close_connection(fd, true);/* exceed size message */
					return;
				}
				else
				{
					byte_buffer_resize(c->rbb, msg_size);
				}
				break;
			}
			if (c->rbb->current < msg_size) { break; }

			handler_post_message(c, (nullmsg_t*)&c->rbb->buf[0]);

			if (c->rbb->current > msg_size)
			{
				memmove(&c->rbb->buf[0], &c->rbb->buf[msg_size], c->rbb->current - msg_size);
			}
			c->rbb->current -= msg_size;
			c->last_recv_time = time(0);
		}
	}
}

static void write_connection_handler(event_loop_t* el, int fd, void* data, int mask)
{
	connection_t* c = shard->conntable[fd];
	if (!c)
	{
		return;/* connection has closed */
	}
	assert(c == data);
	if (c->need_close)
	{
		close_connection(fd, false);/* force close */
		return;
	}
	if (c->wwbb->current == 0 && c->wrbb->current > 0)
	{
		byte_buffer_t* bb = c->wwbb;
		spinlock_lock(&c->wlocker);
		c->wwbb = c->wrbb;
		c->wrbb = bb;
		spinlock_unlock(&c->wlocker);
	}
	while (!shard->halt && c->wwbb->current > 0)
	{
		ssize_t retval = send(fd, &c->wwbb->buf[0], c->wwbb->current, 0);
		if (retval == 0)
		{
			close_connection(fd, true);/* lost connection */
			return;
		}
		if (retval < 0)
		{
			if (interrupted()) { continue; }
			if (wouldblock())
			{
				break;
			}
			close_connection(fd, true);/* send error */
			return;
		}
		else
		{
			size_t remain = byte_buffer_remain(c->wwbb) - (size_t)retval;
			if (remain > 0)
			{
				memmove(&c->wwbb->buf[0], &c->wwbb->buf[(size_t)retval], remain);
			}
			c->wwbb->current -= (size_t)retval;
		}	
	}
	/* Note: on the ET mode, need not delete event
	if (c->wwbb->current == 0)
	{
		file_event_delete(el, c->fd, EVENT_MASK_WRITABLE);
	}
	*/
}

static void accept_connection_handler(event_loop_t* el, int fd, void* data, int mask) 
{
	while (!shard->halt)
	{
		connection_t* c = NULL;
		struct sockaddr_in clientaddr;
		socklen_t cliaddrlen = sizeof(clientaddr);
		int connfd = accept(fd, (struct sockaddr *)&clientaddr, &cliaddrlen);
		if (connfd < 0) 
		{
			if (wouldblock()) { break; }
			if (interrupted())
			{
				continue;
			}
			error_log("accept:%d,%s\n", errno, errstring(errno)); break;
		}

		c = connection_create(connfd, NULL);
		if (!c)
		{
			continue;
		}

		if (!file_event_create(el, connfd, EVENT_MASK_READABLE, read_connection_handler, c))
		{
			connection_delete(c);
			continue;
		}

		assert(connfd < shard->capacity && shard->conntable[connfd] == NULL);
		shard->conntable[connfd] = c;
	}
}

/* fd create_server(ip, port) */
static int cc_create_server(lua_State* L)
{
	int fd, port;
	const char* ip;
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

	/* create & listen port */
	fd = listening_create(ip, port);
	if (fd < 0)
	{
		return 0;
	}

	if (!file_event_create(shard->el, fd, EVENT_MASK_READABLE, accept_connection_handler, NULL)) 
	{
		close(fd);
		return 0;
	}

	lua_pushinteger(L, fd);

	return 1;
}

/* fd create_client(ip, port) */
static int cc_create_client(lua_State* L)
{
	int port;
	const char* ip;
	client_t* client;
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

	client = client_create(handler);
	if (!client)
	{
		return 0;
	}

	if (!client_connect(client, ip, port, NULL))
	{
		client_delete(client);
		return 0;
	}

	if (!file_event_create(shard->el, client->c->fd, EVENT_MASK_READABLE, read_connection_handler, client->c))
	{
		return 0;
	}

	hash_table_insert(handler->client_table, client->c->fd, client);

	assert(shard->conntable[client->c->fd] == NULL);
	shard->conntable[client->c->fd] = client->c;

	lua_pushinteger(L, client->c->fd);

	return 1;
}

static void postmsg(int fd, nullmsg_t* msg)
{
	connection_t* c = NULL;
	if (fd > 0 && fd < shard->capacity)
	{
		c = shard->conntable[fd];
	}
	else
	{
		error_log("illegue fd:%d when sendmsg\n", fd);
		return;
	}
	if (!c)
	{
		error_log("fd:%d not exist or closed\n", fd);
		return;
	}
	if (msg)
	{
		spinlock_lock(&c->wlocker);
		byte_buffer_resize(c->wrbb, msg->len);
		memcpy(byte_buffer_wb(c->wrbb), msg, msg->len);
		c->wrbb->current += msg->len;
		spinlock_unlock(&c->wlocker);
	}
	else
	{
		c->need_close = true;
	}
	/* wake up epoll_wait */
	file_event_create(shard->el, fd, EVENT_MASK_WRITABLE, write_connection_handler, c);
}

/* void sendmsg(fd, userid, cmd, o) */
static int cc_sendmsg(lua_State* L)
{
	int fd, cmd;
	size_t size;
	BYTE buffer[MAX_NETWORK_PACKAGE_SIZE];
	nullmsg_t* msg;
	uint64_t userid;
	worker_handler_t* handler = find_handler_by_stack(L);
	int args = lua_gettop(L);
	if (args < 3)
	{
		error_log("`%s` parameter lack:%d\n", __FUNCTION__, args);
		return 0;
	}

	/* fd */
	if (!lua_isnumber(L, -args))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -args)));
		return 0;
	}

	/* userid */
	if (!lua_isnumber(L, -(args-1)))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -(args-1))));
		return 0;
	}

	/* cmd */
	if (!lua_isnumber(L, -(args-2)))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -(args-2))));
		return 0;
	}

	/* o */
	if (!lua_istable(L, -(args-3)))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -(args-3))));
		return 0;
	}

	fd = lua_tointeger(L, -args);
	userid = lua_tointeger(L, -(args-1)); /* Note: expect lua 5.3 for int64_t */
	cmd = lua_tointeger(L, -(args-2));
	size = sizeof(buffer) - sizeof(nullmsg_t);
	if (!lua_pb_protocol_encode(handler->pb, L, cmd, buffer + sizeof(nullmsg_t), &size))
	{
		error_log("`%s` protocol encode error, cmd:%d\n", __FUNCTION__, cmd);
		return 0;
	}

	debug_log("`%s` fd:%d, userid:%lx, cmd:%d, size:%ld\n", __FUNCTION__, fd, userid, cmd, size);

	msg = (nullmsg_t*)buffer;
	msg->cmd = cmd;
	msg->len = size + sizeof(nullmsg_t);
	msg->userid = userid;
	postmsg(fd, msg);

	return 0;
}

/* void close(fd) */
static int cc_close(lua_State* L)
{
	int fd;
	int args = lua_gettop(L);
	if (args < 1)
	{
		error_log("`%s` parameter lack:%d\n", __FUNCTION__, args);
		return 0;
	}

	if (!lua_isnumber(L, -args))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -args)));
		return 0;
	}

	/* Todo: fd == client ?? */

	fd = lua_tointeger(L, -args);
	postmsg(fd, NULL);

	return 0;
}

void lua_stack_register_network_functions(lua_stack_t* stack)
{
	lua_stack_register_function(stack, "create_server", cc_create_server);	/* fd create_server(ip, port) */
	lua_stack_register_function(stack, "create_client", cc_create_client);	/* fd create_client(ip, port) */
	/* Todo: reconnect_client*/
	lua_stack_register_function(stack, "close", cc_close);					/* void close(fd) */
	lua_stack_register_function(stack, "sendmsg", cc_sendmsg);				/* void sendmsg(fd, userid, cmd, o) */
}

