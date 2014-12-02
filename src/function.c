/*
 * \file: function.c
 * \brief: Created by hushouguo at Nov 07 2014 10:09:42
 */

#include "zebra.h"

static const char* light_key = "@^2A-PK.645*&a=PP\\\"'y#";

const char* domain_lookup(const char* domain)
{
	static char __address_buf[32];
	struct hostent* p = gethostbyname(domain);
	if (!p)
	{
		error_log("lookup domain:%s failure\n", domain);
		return NULL;
	}

	switch (p->h_addrtype)
	{
		case AF_INET:
			{
				char** pp = p->h_addr_list;
				for (; *pp; ++pp)
				{
					//debug_log("\tip:%s\n", inet_ntop(p->h_addrtype, *pp, __address_buf, sizeof(__address_buf)));
					return inet_ntop(p->h_addrtype, *pp, __address_buf, sizeof(__address_buf));
				}
			}
			break;
		case AF_INET6: 
			error_log("NOT SUPPORT IPV6 ADDRESS\n"); 
			break;
		default: 
			error_log("UNKNOWN ADDRESS FORMAT\n");
	}

	return NULL;	
}

void* encrylight(void* p, size_t size)
{
	char *s = (char*)p, *key = (char*)light_key;

	while(size-- > 0)
	{
		*s++ ^= *key++;
		if (*key == '\0') { key = (char*)light_key; }
	}

	return p;
}

void* decrylight(void* p, size_t size)
{
	return encrylight(p, size);
}

bool check_system_limits()
{
	struct rlimit limit;
	if (getrlimit(RLIMIT_CORE, &limit))
	{
		error_log("cannot get RLIMIT_CORE:%s\n", errstring(errno));
		return false;
	}
	debug_log("CORE LIMIT:%ld\n", limit.rlim_max);
	if (limit.rlim_max < 10*MB)
	{
		alarm_log("CORE LIMIT:%ld too small, you should run `ulimit -c` to setting\n", limit.rlim_max);
	}

	if (getrlimit(RLIMIT_NOFILE, &limit))
	{
		error_log("cannot get RLIMIT_NOFILE:%s\n", errstring(errno));
		return false;
	}	
	debug_log("NOFILE LIMIT: rlim_cur:%ld, rlim_max:%ld\n", limit.rlim_cur, limit.rlim_max);
	if (limit.rlim_max < 10000)
	{
		alarm_log("NOFILE LIMIT:%ld too small, you should run `ulimit -n` to setting\n", limit.rlim_max);
	}
	
	if (getrlimit(RLIMIT_STACK, &limit))
	{
		error_log("cannot get RLIMIT_STACK:%s\n", errstring(errno));
		return false;
	}
	debug_log("STACK LIMIT: rlim_cur:%ld, rlim_max:%ld\n", limit.rlim_cur/1024, limit.rlim_max/1024);
	if (limit.rlim_max < 100*MB)
	{
		alarm_log("STACK LIMIT:%ld too small, you should run `ulimit -s` to setting\n", limit.rlim_max);
	}

	return true;
}

