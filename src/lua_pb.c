/*
 * \file: lua_pb.c
 * \brief: Created by hushouguo at Nov 07 2014 03:14:04
 */

#include "zebra.h"

lua_pb_t* lua_pb_create(const char* file)
{
	lua_pb_t* pb = (lua_pb_t*)malloc(sizeof(lua_pb_t));
	memset(pb, 0, sizeof(lua_pb_t));
	pb->pb_package_table = hash_table_create(512);
	if (!lua_pb_package_init(pb, file))
	{
		lua_pb_delete(pb);
		return NULL;
	}
	return pb;
}

void lua_pb_delete(lua_pb_t* pb)
{
	if (pb)
	{
		hash_table_delete(pb->pb_package_table);
		SAFE_FREE(pb);
	}
}

