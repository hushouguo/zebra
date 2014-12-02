/*
 * \file: event_loop.h
 * \brief: Created by hushouguo at 04:36:23 Jun 08 2014
 */
 
#ifndef __EVENT_LOOP_H__
#define __EVENT_LOOP_H__

#define EVENT_MASK_NONE 	0
#define EVENT_MASK_READABLE 1
#define EVENT_MASK_WRITABLE 2
#define EVENT_MASK_ALL		(EVENT_MASK_READABLE|EVENT_MASK_WRITABLE)

struct event_loop;

typedef void event_loop_callback(struct event_loop* el, int fd, void* data, int mask);

/* register file event */
typedef struct r_file_event 
{
	int 					mask; 		/* one of EVENT_(READABLE|WRITABLE) */
	event_loop_callback*	rcallback;
	event_loop_callback*	wcallback;
	void*					data;
} r_file_event_t;

/* state of an event based program */
typedef struct event_loop 
{
	int 					maxfd;   	/* highest file descriptor currently registered */
	int 					capacity; 	/* max number of file descriptors tracked */
	r_file_event_t*			revents; 	/* registered events */
	int						stop;
    int 					epfd;
    struct epoll_event*		events;
} event_loop_t;

event_loop_t* event_loop_create(int capacity);
void event_loop_delete(event_loop_t* el);
int event_loop_poll(event_loop_t* el, int milliseconds);

bool file_event_create(event_loop_t* el, int fd, int mask, event_loop_callback* callback, void*	data);
void file_event_delete(event_loop_t* el, int fd, int mask);

#endif
