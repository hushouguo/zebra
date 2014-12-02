/*
 * \file: main.c
 * \brief: Created by hushouguo at Thu 16 Oct 2014 10:36:28 AM CST
 */

#include "zebra.h"

#define LOGGER_APP_NAME			"zebra"
#define MIN_FRAME_INTERVAL		100
#define DEF_CONNECTION_EXPIRE	60

static void signal_shutdown_handler(int sig) 
{
    switch (sig) 
	{
		case SIGHUP:
			/* Todo: reload lua script */
			break;
		case SIGINT:
		case SIGTERM:
		case SIGQUIT:
			shard->halt = true;
			debug_log("Received signal:%d, scheduling shutdown...\n", sig);
			break;
		default:
			debug_log("Received signal:%d\n", sig); break;
    };
}

static void init_signal_handler(void) 
{
    struct sigaction act;

    /*signal(SIGHUP, SIG_IGN);*/
    signal(SIGPIPE, SIG_IGN);

    /* When the SA_SIGINFO flag is set in sa_flags then sa_sigaction is used.
     * Otherwise, sa_handler is used. */
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = signal_shutdown_handler;
    sigaction(SIGHUP, &act, NULL);
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTERM, &act, NULL);
    sigaction(SIGQUIT, &act, NULL);
    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGUSR2, &act, NULL);
    sigaction(SIGILL, &act, NULL);
}

void usage()
{
	debug_log("Usage: shard [OPTIONS]\n");
	debug_log("         OPTIONS:  \n");
	debug_log("           -d: become daemon, [%s]\n", DEF_SHARD_DAEMON ? "true" : "false");
	debug_log("           -h: show help information\n");
	debug_log("           -c: setting capacity of shard, [%d]\n", DEF_SHARD_CAPACITY);
	debug_log("           -n: specify thread number, [%d]\n", DEF_SHARD_POOL_SIZE);
	debug_log("           -r: specify main script file\n");
	debug_log("           -t: specify lua script tick milliseconds, [%d]\n", DEF_TICK_MILLISECONDS);
	debug_log("           -l: specify protocol file, [%s]\n", DEF_PROTOCOL_FILE);
}

bool parse_args(int argc, char* argv[])
{
    int c;
	char* suffix;

	/*--argc, argv++;*/

    while((c = getopt(argc, argv, "dDhH:c:n:r:t:l:")) != -1) 
    {   
        switch(c)
        {   
            case 'd': case 'D': shard->daemon = true; break;
			case 'h': usage(); return false;
			case 'c': shard->capacity = atoi(optarg); break;
			case 'n': shard->pool_size = atoi(optarg); break;
			case 'r': shard->main_script_file = strdup(optarg); break;
			case 't': shard->tick_milliseconds = atoi(optarg); break;
			case 'l': shard->protocol = strdup(optarg); break;
			default: usage(); return false;
        }
    }

	if (shard->pool_size == 0)
	{
		error_log("at very least need one thread\n");
		return false;
	}

	if (!shard->main_script_file)
	{
		error_log("MUST specify `main script file`\n");
		return false;
	}

	if (!shard->protocol)
	{
		shard->protocol = strdup(DEF_PROTOCOL_FILE);
	}

	/* truncate .lua suffix*/
	suffix = strrchr(shard->main_script_file, '.');
	if (suffix && strcasecmp(suffix, ".lua") == 0)
	{
		*suffix = '\0';
	}

    return true;
} 

static void handler_runnable(void* p)
{
	size_t n;
	int nummsg = 0, fd, elapse_milliseconds;
	nullmsg_t* msg;
	struct timespec begintime, endtime, frametime;
	worker_handler_t* handler = (worker_handler_t*)p;

	clock_gettime(CLOCK_REALTIME, &begintime);
	lua_protocol_start(handler->stack, handler->id);
	while (!shard->halt)
	{
		clock_gettime(CLOCK_REALTIME, &frametime);

		/* handle network message */
		while (1)
		{
			msg = msg_queue_pop(handler->q, &fd);
			if (!msg)
			{
				break;
			}
			++nummsg;
			if (msg->len == 0)
			{
				/* connection close */
				lua_protocol_connection_close(handler->stack, fd);
			}
			else
			{
				lua_protocol_msg_parser(handler->stack, handler->pb, fd, msg);
			}
			msg_queue_free(handler->q, msg);
		}

		/* handle http request */
		for (n = 0; n < handler->http_table->nodesize; ++n)
		{
			hash_node_t* node = handler->http_table->nodes[n];
			while (node)
			{
				http_loop((http_t*)node->value);
				node = node->next;
			}
		}

		clock_gettime(CLOCK_REALTIME, &endtime);
		elapse_milliseconds = (endtime.tv_sec - begintime.tv_sec)*1000 + (endtime.tv_nsec - begintime.tv_nsec)/1000000;

		/* DEF 100 milliseconds time tick */
		if (elapse_milliseconds >= DEF_TICK_MILLISECONDS)
		{
			begintime = endtime;
			lua_protocol_timetick(handler->stack, elapse_milliseconds);
		}

		elapse_milliseconds = (endtime.tv_sec - frametime.tv_sec)*1000 + (endtime.tv_nsec - frametime.tv_nsec)/1000000;
		if (elapse_milliseconds < MIN_FRAME_INTERVAL)
		{
			usleep((MIN_FRAME_INTERVAL - elapse_milliseconds)*1000);
		}
		else { sched_yield(); }
	}
	lua_protocol_stop(handler->stack);
}

extern void close_connection(int, bool);

int main(int argc, char* argv[])
{
	int n;

	init_signal_handler();

	if (!parse_args(argc, argv)) { return 1; }

	if (!check_system_limits()) { return 1; }

	/* create crontab */
	shard->crontab = crontab_create();
	if (!shard->crontab) { return 1; }

	/* create event loop */
	shard->conntable = (connection_t**)malloc(shard->capacity * sizeof(connection_t*));
	if ((shard->el = event_loop_create(shard->capacity)) == NULL) { return 1; }

	/* create handler pool */
	shard->worker_handler_pool = (worker_handler_t**)malloc(sizeof(worker_handler_t*) * shard->pool_size);
	for (n = 0; n < shard->pool_size && !shard->halt; ++n)
	{
		worker_handler_t* worker_handler = shard->worker_handler_pool[n] = handler_create(n);
		if (!worker_handler)
		{
			return 1;
		}
		thread_pool_insert("handler", handler_runnable, worker_handler);
	}

	while(!shard->halt)
	{
		time_t nowtime = time(0);
		event_loop_poll(shard->el, -1);
		for (n = 0; n < shard->capacity; ++n)
		{
			connection_t* c = shard->conntable[n];
			if (c && !c->client && (nowtime - c->last_recv_time) >= DEF_CONNECTION_EXPIRE)
			{
				close_connection(c->fd, false);
			}
		}
		sched_yield();
	}

	thread_pool_exit();

	debug_log("Bye!\n");
	return 0;
}

