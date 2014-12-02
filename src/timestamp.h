/*
 * \file: timestamp.h
 * \brief: Created by hushouguo at Thu 16 Oct 2014 11:10:01 AM CST
 */
 
#ifndef __TIMESTAMP_H__
#define __TIMESTAMP_H__

/* Non thread-safe function */
const char* timestamp(time_t t, const char* format);/* = "%y/%02m/%02d %02H:%02M:%02S");*/

#endif
