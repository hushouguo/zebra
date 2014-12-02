/*
 * \file: lua_pb_protocol.h
 * \brief: Created by hushouguo at Nov 07 2014 05:13:33
 */
 
#ifndef __LUA_PB_PROTOCOL_H__
#define __LUA_PB_PROTOCOL_H__

struct lua_pb;
bool lua_pb_protocol_decode(struct lua_pb* pb, lua_State* L, int cmd, BYTE* b, size_t size);
bool lua_pb_protocol_encode(struct lua_pb* pb, lua_State* L, int cmd, BYTE* b, size_t* size);

#endif
