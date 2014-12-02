/*
 * \file: lua_protocol.c
 * \brief: Created by hushouguo at Nov 06 2014 03:06:57
 */

#include "zebra.h"

bool lua_protocol_start(lua_stack_t* stack, int n)
{
	if (lua_stack_push_global_function(stack, __FUNCTION__))
	{
		lua_pushinteger(stack->L, n);
		lua_stack_execute_function(stack, 1);
		lua_stack_clean(stack);
		return true;
	}
	return false;
}

bool lua_protocol_stop(lua_stack_t* stack)
{
	if (lua_stack_push_global_function(stack, __FUNCTION__))
	{
		lua_stack_execute_function(stack, 0);
		lua_stack_clean(stack);
		return true;
	}
	return false;
}

bool lua_protocol_connection_close(lua_stack_t* stack, int fd)
{
	if (lua_stack_push_global_function(stack, __FUNCTION__))
	{
		lua_pushinteger(stack->L, fd);
		lua_stack_execute_function(stack, 1);
		lua_stack_clean(stack);
		return true;
	}
	return false;
}

bool lua_protocol_msg_parser(lua_stack_t* stack, lua_pb_t* pb, int fd, nullmsg_t* msg)
{
	BYTE* b;
	size_t size;
	if (lua_stack_push_global_function(stack, __FUNCTION__))
	{
		lua_pushinteger(stack->L, fd);
		lua_pushinteger(stack->L, msg->userid);/* Note: expect lua 5.3 for int64_t */
		lua_pushinteger(stack->L, msg->cmd);
		b = (BYTE*)msg + sizeof(nullmsg_t);
		size = msg->len - sizeof(nullmsg_t);
		if (lua_pb_protocol_decode(pb, stack->L, msg->cmd, b, size))
		{
			lua_stack_execute_function(stack, 4);
		}
		lua_stack_clean(stack);
		return true;
	}
	return false;
}

bool lua_protocol_timetick(lua_stack_t* stack, int milliseconds)
{
	if (lua_stack_push_global_function(stack, __FUNCTION__))
	{
		lua_pushinteger(stack->L, milliseconds);
		lua_stack_execute_function(stack, 1);
		lua_stack_clean(stack);
		return true;
	}
	return false;
}

bool lua_protocol_http_request(lua_stack_t* stack, int httpid, int reqid, const char* uri)
{
	if (lua_stack_push_global_function(stack, __FUNCTION__))
	{
		lua_pushinteger(stack->L, httpid);
		lua_pushinteger(stack->L, reqid);
		lua_pushstring(stack->L, uri);
		lua_stack_execute_function(stack, 3);
		lua_stack_clean(stack);
		return true;
	}
	return false;
}

