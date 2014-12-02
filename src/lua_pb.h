/*
 * \file: lua_pb.h
 * \brief: Created by hushouguo at Nov 07 2014 03:10:59
 */
 
#ifndef __LUA_PB_H__
#define __LUA_PB_H__

typedef struct lua_pb
{
	hash_table_t*	pb_package_table;
} lua_pb_t;

lua_pb_t* lua_pb_create(const char* file);
void lua_pb_delete(lua_pb_t* pb);

#endif
