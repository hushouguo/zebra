/*
 * \file: msg_queue.c
 * \brief: Created by hushouguo at Nov 05 2014 18:04:13
 */

#include "zebra.h"

msg_queue_t* msg_queue_create()
{
	msg_queue_t* q = (msg_queue_t*)malloc(sizeof(msg_queue_t));
	memset(q, 0, sizeof(msg_queue_t));
	q->rq = (message_queue_t*)malloc(sizeof(message_queue_t));
	q->rq->header = q->rq->tailer = NULL;
	q->wq = (message_queue_t*)malloc(sizeof(message_queue_t));
	q->wq->header = q->wq->tailer = NULL;
	q->qlocker = 0;
	q->rq_num = q->wq_num = 0;
	return q;
}

void msg_queue_delete(msg_queue_t* q)
{
	if (q)
	{
		/*Todo: free msg*/
		SAFE_FREE(q->rq);
		SAFE_FREE(q->wq);
		SAFE_FREE(q);
	}
}

void msg_queue_push(msg_queue_t* q, int fd, nullmsg_t* msg)
{
	message_t* newmsg = (message_t*)malloc(sizeof(message_t) + (msg ? msg->len : sizeof(nullmsg_t)));
	newmsg->next = NULL;
	newmsg->fd = fd;
	if (msg)
	{
		memcpy(newmsg->msg, msg, msg->len);
	}
	else
	{
		memset(newmsg->msg, 0, sizeof(nullmsg_t));
	}
	spinlock_lock(&q->qlocker);
	if (q->wq->tailer) { q->wq->tailer->next = newmsg; }
	q->wq->tailer = newmsg;
	if (!q->wq->header) { q->wq->header = newmsg; }
	spinlock_unlock(&q->qlocker);
	++q->rq_num;
}

nullmsg_t* msg_queue_pop(msg_queue_t* q, int* fd)
{
	nullmsg_t* msg = NULL;
	if (!q->rq->header && q->wq->header)
	{
		message_queue_t* temp = q->rq;
		q->rq = q->wq;
		spinlock_lock(&q->qlocker);
		q->wq = temp;
		spinlock_unlock(&q->qlocker);
	}
	if (q->rq->header)
	{
		message_t* newmsg = q->rq->header;
		q->rq->header = q->rq->header->next;
		if (!q->rq->header) { q->rq->tailer = NULL; }
		*fd = newmsg->fd;
		msg = (nullmsg_t*)newmsg->msg;
	}
	if (msg) { ++q->wq_num; }
	return msg;
}

bool msg_queue_empty(msg_queue_t* q)
{
	return q->rq->header == NULL;
}

void msg_queue_free(msg_queue_t* q, nullmsg_t* msg)
{
	message_t* newmsg = (message_t*)((size_t)msg - offsetof(message_t, msg));
	SAFE_FREE(newmsg);	
}

size_t msg_queue_size(msg_queue_t* q)
{
	return q->rq_num - q->wq_num;
}

