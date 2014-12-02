/*
 * \file: errstr.c
 * \brief: Created by hushouguo at Thu 16 Oct 2014 10:58:01 AM CST
 */

#include "zebra.h"

#define SYS_ERRNO	128

static char** errlist = NULL;

static void errstring_init()
{
	int err;
	errlist = (char **)malloc(SYS_ERRNO * sizeof(char*));
	for (err = 0; err < SYS_ERRNO; ++err)
	{
		errlist[err] = strdup(strerror(err));
	}
}

const char* errstring(int err)
{
/*#ifdef _WIN32*/
	if (!errlist) { errstring_init(); }
/*#endif*/
	return err >= 0 && err < SYS_ERRNO ? errlist[err] : "Unknown error";
}

