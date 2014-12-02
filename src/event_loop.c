/*
 * \file: event_loop.c
 * \brief: Created by hushouguo at 03:34:44 Oct 01 2013
 */

#include "zebra.h"

event_loop_t* event_loop_create(int capacity)
{
	int i;
	event_loop_t* el = (event_loop_t*)malloc(sizeof(event_loop_t));
	if (!el) { return NULL; }
	memset(el, 0, sizeof(event_loop_t));
	el->revents = (r_file_event_t*)malloc(sizeof(r_file_event_t)*capacity);
	if (!el->revents) { return NULL; }
	el->capacity = capacity;
	el->stop = 0;
	el->maxfd = -1;
	el->events = (struct epoll_event*)malloc(sizeof(struct epoll_event)*capacity);
	if (!el->events) { return NULL; }
	el->epfd = epoll_create(1024); /* 1024 is just a hint for the kernel */
	if (el->epfd < 0) { return NULL; }
    for (i = 0; i < capacity; ++i)
	{
		el->revents[i].mask = EVENT_MASK_NONE;
	}
	return el;
}
void event_loop_delete(event_loop_t* el)
{
	if (el)
	{
		close(el->epfd);
		SAFE_FREE(el->events);
		SAFE_FREE(el->revents);
		SAFE_FREE(el);
	}
}
bool file_event_create(event_loop_t* el, int fd, int mask, event_loop_callback* callback, void* data)
{
	r_file_event_t* revent;
	struct epoll_event ee;
	if (fd >= el->capacity) 
	{
		error_log("max capacity of file event reached:%d\n", el->capacity);
		return false; 
	}
	revent = &el->revents[fd];
	if (mask & EVENT_MASK_READABLE) { revent->rcallback = callback; }
	if (mask & EVENT_MASK_WRITABLE) { revent->wcallback = callback; }
	revent->data = data;
	mask |= revent->mask; /* merge old events */
    ee.events = EPOLLET;
    if (mask & EVENT_MASK_READABLE) { ee.events |= EPOLLIN; }
    if (mask & EVENT_MASK_WRITABLE) { ee.events |= EPOLLOUT; }
    ee.data.u64 = 0; /* avoid valgrind warning */
    ee.data.fd = fd;
    /* If the fd was already monitored for some event, we need a MOD operation. Otherwise we need an ADD operation. */
    if (epoll_ctl(el->epfd, revent->mask == EVENT_MASK_NONE ? EPOLL_CTL_ADD : EPOLL_CTL_MOD, fd, &ee) == -1)
	{
		error_log("epoll_ctl failure:%d,%s\n", errno, errstring(errno));
		return false; 
	}
	revent->mask |= mask;
	if (fd > el->maxfd ) { el->maxfd = fd; }
    return true;
}
void file_event_delete(event_loop_t* el, int fd, int mask)
{
	r_file_event_t* revent;
    struct epoll_event ee;
	if (fd >= el->capacity) { return; }
	revent = &el->revents[fd];
	if (revent->mask == EVENT_MASK_NONE) { return; }
	mask = revent->mask & (~mask);
    ee.events = EPOLLET;
    if (mask & EVENT_MASK_READABLE) { ee.events |= EPOLLIN; }
    if (mask & EVENT_MASK_WRITABLE) { ee.events |= EPOLLOUT; }
    ee.data.u64 = 0; /* avoid valgrind warning */
    ee.data.fd = fd;
    if (mask != EVENT_MASK_NONE) 
	{
        epoll_ctl(el->epfd, EPOLL_CTL_MOD, fd, &ee);
    } 
	else 
	{
        /* Note, Kernel < 2.6.9 requires a non null event pointer even for EPOLL_CTL_DEL. */
        epoll_ctl(el->epfd, EPOLL_CTL_DEL, fd, &ee);
    }
	revent->mask = mask;
}
int event_loop_poll(event_loop_t* el, int milliseconds)
{
    int i, numevents = epoll_wait(el->epfd, el->events, el->capacity, milliseconds);
	if (numevents < 0)
	{
		if (errno != EINTR)
		{
			debug_log("epoll wait error:%d, %s\n", errno, errstring(errno));
		}
	}
	for (i = 0; i < numevents; ++i) 
	{
		struct epoll_event* ee = &el->events[i];
		r_file_event_t* revent = &el->revents[ee->data.fd]; 
		if (ee->events & EPOLLIN) 
		{ 
			if (revent->rcallback) { revent->rcallback(el, ee->data.fd, revent->data, revent->mask); }
		}
		if ((ee->events & EPOLLOUT) || (ee->events & EPOLLERR) || (ee->events & EPOLLHUP)) 
		{ 
			if (ee->events & EPOLLERR)
			{
				error_log("fd:%d receive EPOLLERR\n", ee->data.fd);
			}
			if (ee->events & EPOLLHUP)
			{
				error_log("fd:%d receive EPOLLHUP\n", ee->data.fd);
			}
			if (revent->wcallback) { revent->wcallback(el, ee->data.fd, revent->data, revent->mask); }
		}
	}
	return numevents;
}

