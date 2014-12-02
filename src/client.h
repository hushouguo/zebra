/*
 * \file: client.h
 * \brief: Created by hushouguo at Nov 07 2014 10:26:08
 */
 
#ifndef __CLIENT_H__
#define __CLIENT_H__

#define MAX_DOMAIN_SIZE		128
#define DEF_CLIENT_CONNECT_EXPIRE	10

struct worker_handler;
typedef struct client
{
	char					ip[16];
	int						port;
	char					domain[MAX_DOMAIN_SIZE];
	time_t					retry_time;
	connection_t*			c;
	struct worker_handler*	owner;
} client_t;

client_t* client_create(struct worker_handler* handler);
void client_delete(client_t* client);
bool client_connect(client_t* client, const char* ip, int port, const char* domain);
bool client_reconnect(client_t* client);

#endif
