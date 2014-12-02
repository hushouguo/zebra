/*
 * \file: byte_buffer.h
 * \brief: Created by hushouguo at Thu 16 Oct 2014 10:25:29 PM CST
 */
 
#ifndef __BYTE_BUFFER_H__
#define __BYTE_BUFFER_H__

typedef struct byte_buffer
{
	char*			buf;
	size_t			len;
	size_t			current;
} byte_buffer_t;

byte_buffer_t* byte_buffer_create(size_t prealloc_size);
void byte_buffer_delete(byte_buffer_t* bb);
void byte_buffer_resize(byte_buffer_t* bb, size_t len);
size_t byte_buffer_remain(byte_buffer_t* bb);
char* byte_buffer_wb(byte_buffer_t* bb);
void byte_buffer_push_char(byte_buffer_t* bb, char c);
char byte_buffer_pop_char(byte_buffer_t* bb);
void byte_buffer_push_data(byte_buffer_t* bb, char* p, size_t elems);
void byte_buffer_reset(byte_buffer_t* bb);

#endif
