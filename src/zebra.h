/*
 * \file: zebra.h
 * \brief: Created by hushouguo at Thu 16 Oct 2014 11:35:21 AM CST
 */
 
#ifndef __ZEBRA_H__
#define __ZEBRA_H__

#if defined(_WIN32)
#pragma warning(disable:4200)
#pragma warning(disable:4819)
#endif

typedef unsigned char					BYTE;

#if !defined(__cplusplus)
typedef int								bool;
#define true							1
#define false							0
#endif

#define min(a,b) 						(((a) > (b)) ? (b) : (a))
#define max(a,b) 						(((a) > (b)) ? (a) : (b))

#define	KB								1024U
#define MB								1048576U
#define GB								1073741824U
#define TB								1099511627776ULL

#define MINUTE							60U
#define HOUR							3600U
#define DAY								86400U

#ifndef PATH_MAX
	#define PATH_MAX					255
#endif

#define SAFE_FREE(P)					do { if(P) { free(P); (P) = 0; } } while(0)

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include <stddef.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netdb.h>
#ifdef LINUX
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <linux/limits.h>
#include <pwd.h>
#include <execinfo.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#endif

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include <mysql/mysql.h>
#include <mysql/errmsg.h>

#include <curl/curl.h>

#include "libevent/event.h"
#include "libevent/evhttp.h"

#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"

#include "luacurl/Lua-cURL.h"
#include "luacurl/Lua-cURL-share.h"
#include "luacurl/Lua-utility.h"

#include "jemalloc/jemalloc.h"

#include "byte_buffer.h"
#include "errstring.h"
#include "timestamp.h"
#include "random.h"
#include "function.h"
#include "hash_table.h"
#include "xml_parser.h"
#include "configure.h"
#include "crontab.h"
#include "json_parser.h"
#include "thread.h"
#include "lua_stack.h"
#include "logger.h"
#include "record.h"
#include "nullmsg.h"
#include "msg_queue.h"
#include "networking.h"
#include "event_loop.h"
#include "listening.h"
#include "connection.h"
#include "lua_pb.h"
#include "lua_protocol.h"
#include "lua_xml_parser.h"
#include "lua_json_parser.h"
#include "lua_pb_package.h"
#include "lua_pb_protocol.h"
#include "client.h"
#include "http.h"
#include "worker_handler.h"
#include "shard.h"

#endif
