/*
 * \file: byte_buffer.c
 * \brief: Created by hushouguo at Thu 16 Oct 2014 10:33:05 PM CST
 */

#include "zebra.h"

byte_buffer_t* byte_buffer_create(size_t prealloc_size)
{
	byte_buffer_t* bb = (byte_buffer_t*)malloc(sizeof(byte_buffer_t));
	memset(bb, 0, sizeof(byte_buffer_t));
	bb->buf = NULL;
	bb->len = bb->current = 0;
	if (prealloc_size > 0)
	{
		bb->buf = (char*)malloc(prealloc_size);
		bb->len = prealloc_size;
	}
	return bb;
}
void byte_buffer_delete(byte_buffer_t* bb)
{
	if (bb)
	{
		SAFE_FREE(bb->buf);
		SAFE_FREE(bb);
	}
}
void byte_buffer_resize(byte_buffer_t* bb, size_t len)
{
	size_t remain = byte_buffer_remain(bb);
	if (remain < len)
	{
		len += bb->len;
		bb->buf = (char*)realloc(bb->buf, len);
		bb->len = len;
	}
}
size_t byte_buffer_remain(byte_buffer_t* bb)
{
	return bb->len - bb->current;
}
char* byte_buffer_wb(byte_buffer_t* bb)
{
	return &bb->buf[bb->current];
}
void byte_buffer_push_char(byte_buffer_t* bb, char c)
{
	byte_buffer_resize(bb, 1);
	bb->buf[bb->current++] = c;
}
char byte_buffer_pop_char(byte_buffer_t* bb)
{
	assert(bb->current > 0);
	return bb->buf[--bb->current];
}
void byte_buffer_push_data(byte_buffer_t* bb, char* p, size_t elems)
{
	byte_buffer_resize(bb, elems);
	memcpy(byte_buffer_wb(bb), p, elems);
	bb->current += elems;
}
void byte_buffer_reset(byte_buffer_t* bb)
{
	bb->current = 0;
}

