/*
 * \file: thread.h
 * \brief: Created by hushouguo at Nov 05 2014 01:58:25
 */
 
#ifndef __THREAD_H__
#define __THREAD_H__

#if (__i386__ || __i386 || __amd64__ || __amd64)
#define pause() __asm__("pause")
#else
#define pause()
#endif

#if ((__GNUC__ > 4) || (__GNUC__ == 4) && (__GNUC_MINOR__ >= 1))
#define atomic_cmp_set(lock, old, set)	__sync_bool_compare_and_swap(lock, old, set)
#define atomic_fetch_add(value, add)	__sync_fetch_and_add(value, add)
#else
#error("GCC version is too low!")
#endif

typedef volatile long atomic_t;

void spinlock_lock(atomic_t* locker);
bool spinlock_trylock(atomic_t* locker);
void spinlock_unlock(atomic_t* locker);

typedef void (*runnable_t)(void*);

bool thread_pool_insert(const char* name, runnable_t runnable, void* p);
void thread_pool_exit();

#endif
