/*
 * \file: shard.c
 * \brief: Created by hushouguo at Nov 04 2014 23:49:57
 */

#include "zebra.h"

static shard_t static_shard = 
{
	NULL,		/* main_script_file */
	DEF_SHARD_CAPACITY,		/* capacity */
	DEF_SHARD_POOL_SIZE,			/* pool_size */
	NULL,		/* worker_handler_pool */
	NULL,		/* el */
	NULL,		/* conntable */
	DEF_SHARD_DAEMON,		/* daemon */
	false,		/* halt */
	DEF_TICK_MILLISECONDS,	/* tick_milliseconds */
	NULL,		/* protocol file */
	NULL,
};
shard_t* shard = &static_shard;
void halt()
{
	shard->halt = true;
	kill(getpid(), SIGINT);
}
