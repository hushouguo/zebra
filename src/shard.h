/*
 * \file: shard.h
 * \brief: Created by hushouguo at Nov 04 2014 23:49:53
 */
 
#ifndef __SHARD_H__
#define __SHARD_H__

#define DEF_SHARD_CAPACITY						1000
#define DEF_SHARD_POOL_SIZE						8
#define DEF_SHARD_DAEMON						false
#define DEF_TICK_MILLISECONDS					100
#define DEF_PROTOCOL_FILE						"protocol/protocol.xml"

typedef struct shard_struct
{
	char* 										main_script_file;
	int											capacity;
	int											pool_size;
	worker_handler_t**							worker_handler_pool;
	event_loop_t*								el;
	connection_t**								conntable;
	bool										daemon;
	bool										halt;
	int											tick_milliseconds;
	char*										protocol;
	crontab_t*									crontab;
} shard_t;

extern shard_t* shard;

void halt();

#endif
