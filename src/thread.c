/*
 * \file: thread.c
 * \brief: Created by hushouguo at Nov 05 2014 02:00:55
 */

#include "zebra.h"

volatile uint32_t sys_cpus = 1;

void spinlock_lock(atomic_t* locker)
{
	int i, count = 0, n = 0;
	for (;;++count) 
	{
		if (*locker == 0 && atomic_cmp_set(locker, 0, 1)) { return; }
		if (sys_cpus > 1)
		{
			for (n = 1; n < 16; n <<= 1) 
			{
				for (i = 0; i < n; i++) 
				{
					pause(); 
				}				 
				if (*locker == 0 && atomic_cmp_set(locker, 0, 1)) { return; }
			}
		}
		sched_yield();
	}
}
bool spinlock_trylock(atomic_t* locker)
{
	return (*(locker) == 0 && atomic_cmp_set(locker, 0, 1));
}
void spinlock_unlock(atomic_t* locker)
{
	/*atomic_cmp_set(locker, 1, 0);*/
	*locker = 0;
}

typedef struct thread_struct
{
	struct thread_struct*		next;
	const char*					name;
	pthread_t		 			pid;
	pthread_attr_t  			attr;
	size_t 		 				stacksize;
	runnable_t	 				runnable;
	void*			 			p;
	bool			 			running;
} thread_t;

static thread_t* thread_pool = NULL;

static void* static_runnable(void* p)
{
	thread_t* thread = (thread_t*)p;

	/*debug_log("thread:%s is running with stack size:%u\n", thread->name, (uint32_t)thread->stacksize);*/
	debug_log("thread:%s is running\n", thread->name);
	thread->runnable(thread->p);
	debug_log("thread:%s is terminated\n", thread->name);

	thread->running = false;
	pthread_exit(NULL);

	return NULL;
}


thread_t* thread_create(runnable_t runnable, size_t stacksize, const char* name, void* p)
{
	thread_t* thread = (thread_t*)malloc(sizeof(thread_t));
	memset(thread, 0, sizeof(thread_t));

	if (pthread_attr_init(&thread->attr) < 0)
	{
		error_log("pthread_attr_init:%d,%s\n", errno, errstring(errno));
		free(thread); 
		return NULL;
	}

	if (pthread_attr_setstacksize(&thread->attr, stacksize) < 0)
	{
		error_log("pthread_attr_setstacksize:%d,%s\n", errno, errstring(errno));
		free(thread); 
		return NULL;
	}

	if (pthread_attr_getstacksize(&thread->attr, &thread->stacksize) < 0)
	{
		error_log("pthread_attr_getstacksize:%d,%s\n", errno, errstring(errno));
		free(thread);
		return NULL;
	}

	thread->name = name;
	/*thread->stacksize = stacksize;*/
	thread->runnable = runnable;
	thread->p = p;
	thread->running = true;

	if (pthread_create(&thread->pid, &thread->attr, static_runnable, thread))
	{
		error_log("pthread_create:%d,%s\n", errno, errstring(errno));
		free(thread); 
		return NULL;
	}

	return thread;
}
bool thread_pool_insert(const char* name, runnable_t runnable, void* p)
{
	thread_t* thread = thread_create(runnable, 16*MB, name, p);
	if (!thread)
	{
		error_log("cannot create thread:%d,%s\n", errno, errstring(errno));
		return false;
	}
	thread->next = thread_pool;
	thread_pool = thread;
	return true;
}
void thread_pool_exit()
{
	thread_t* thread = thread_pool, *temp = NULL;
	while (thread)
	{
		if (thread->running)
		{
			sched_yield();
			//usleep(100); 
			continue; 
		}
		temp = thread;
		thread = thread->next;
		free(temp);
	}
	thread_pool = NULL;
}
#ifdef LINUX
__attribute__((constructor)) static void thread_pool_init()
{
	int i = sysconf(_SC_NPROCESSORS_CONF);
	if (i < 0)
	{
		error_log("cannot fetch _SC_NPROCESSORS_CONF value\n");
		sys_cpus = 1;
	}
	else
	{
		sys_cpus = i;
	}
	debug_log("found %d cpus\n", sys_cpus);
}
#endif

