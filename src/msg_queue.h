/*
 * \file: msg_queue.h
 * \brief: Created by hushouguo at Nov 05 2014 17:54:28
 */
 
#ifndef __MSG_QUEUE_H__
#define __MSG_QUEUE_H__

#ifndef offsetof
#define offsetof(STRUCTURE,FIELD) ((size_t)((char*)&((STRUCTURE*)0)->FIELD))
#endif

typedef struct message
{
	struct message*		next;
	int					fd;
	char				msg[0];
} message_t;

typedef struct message_queue
{
	message_t*			header;
	message_t*			tailer;
} message_queue_t;

typedef struct msg_queue                   
{
	message_queue_t*	rq;
	message_queue_t*	wq;
	atomic_t			qlocker;
	size_t				rq_num;
	size_t				wq_num;
} msg_queue_t;

msg_queue_t* msg_queue_create();
void msg_queue_delete(msg_queue_t* q);
void msg_queue_push(msg_queue_t* q, int fd, nullmsg_t* msg);
nullmsg_t* msg_queue_pop(msg_queue_t* q, int* fd);
bool msg_queue_empty(msg_queue_t* q);
void msg_queue_free(msg_queue_t* q, nullmsg_t* msg);
size_t msg_queue_size(msg_queue_t* q);

#endif
