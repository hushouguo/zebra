/*
 * \file: nullmsg.h
 * \brief: Created by hushouguo at Thu 16 Oct 2014 10:20:29 PM CST
 */
 
#ifndef __NULLMSG_H__
#define __NULLMSG_H__

#define MAX_NETWORK_PACKAGE_SIZE	64*KB

#pragma pack(push, 1)
typedef struct nullmsg
{
	unsigned short	len;
	unsigned short	cmd;
	uint64_t		userid;
} nullmsg_t;
#pragma pack(pop)

#endif
