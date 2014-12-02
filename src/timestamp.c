/*
 * \file: timestamp.c
 * \brief: Created by hushouguo at Thu 16 Oct 2014 11:10:53 AM CST
 */

#include "zebra.h"

const char* timestamp(time_t t, const char* format)
{
	static char timestamp_buffer[24];
	struct tm tm;
	localtime_r((const time_t*)&t, &tm);
	strftime(timestamp_buffer, sizeof(timestamp_buffer), format, &tm);
	return timestamp_buffer;
}

