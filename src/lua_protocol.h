/*
 * \file: lua_protocol.h
 * \brief: Created by hushouguo at Nov 06 2014 03:04:15
 */
 
#ifndef __LUA_PROTOCOL_H__
#define __LUA_PROTOCOL_H__

bool lua_protocol_start(lua_stack_t* stack, int n);
bool lua_protocol_stop(lua_stack_t* stack);
bool lua_protocol_connection_close(lua_stack_t* stack, int fd);
bool lua_protocol_msg_parser(lua_stack_t* stack, lua_pb_t* pb, int fd, nullmsg_t* msg);
bool lua_protocol_timetick(lua_stack_t* stack, int milliseconds);
bool lua_protocol_http_request(lua_stack_t* stack, int httpid, int reqid, const char* uri);

#endif
