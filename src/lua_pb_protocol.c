/*
 * \file: lua_pb_protocol.c
 * \brief: Created by hushouguo at Nov 07 2014 05:16:18
 */

#include "zebra.h"

#define CHECK_SIZE(size, n)	\
	if (size < n) \
	{ \
		error_log("decode package node error, size:%lu, n:%u\n", size, n); \
		return false; \
	}

#define CHECK_BUFSIZE(size, n, byte) \
	if ((n + byte) > size) \
	{ \
		error_log("encode package node error, size:%lu, n:%lu, desc:%s\n", size, n + byte, node->desc); \
		return false; \
	}

//------------------------------------------- decode ---------------------------------------------------
bool lua_pb_protocol_decode_package_node_v(lua_State* L, lua_pb_package_node_t* node, BYTE** b, size_t* size)
{
	switch(node->value_type)
	{
		case PB_PACKAGE_VALUE_TYPE_CHAR:
			{
				lua_Integer value;
				CHECK_SIZE(*size, 1);
				value = *(int8_t*)*b;
				--*size, ++*b;
				lua_pushstring(L, node->desc);
				lua_pushinteger(L, value);
				lua_settable(L, -3);
			}
			break;
		case PB_PACKAGE_VALUE_TYPE_SHORT:
			{
				lua_Integer value;
				CHECK_SIZE(*size, 2);
				value = *(int16_t*)*b;
				*size -= 2, *b += 2;
				lua_pushstring(L, node->desc);
				lua_pushinteger(L, value);
				lua_settable(L, -3);
			}
			break;
		case PB_PACKAGE_VALUE_TYPE_INT:
			{
				lua_Integer value;
				CHECK_SIZE(*size, 4);
				value = *(int32_t*)*b;
				*size -= 4, *b += 4;
				lua_pushstring(L, node->desc);
				lua_pushinteger(L, value);
				lua_settable(L, -3);
			}
			break;
		case PB_PACKAGE_VALUE_TYPE_LONG:
			{
				lua_Integer value;
				CHECK_SIZE(*size, 8);
				value = *(int64_t*)*b;
				*size -= 8, *b += 8;
				lua_pushstring(L, node->desc);
				lua_pushinteger(L, value);
				lua_settable(L, -3);
			}
			break;
		case PB_PACKAGE_VALUE_TYPE_BOOL:
			{
				lua_Integer value;
				CHECK_SIZE(*size, 1);
				value = *(int8_t*)*b;
				--*size, ++*b;
				lua_pushstring(L, node->desc);
				lua_pushboolean(L, value);
				lua_settable(L, -3);
			}
			break;
		case PB_PACKAGE_VALUE_TYPE_DOUBLE:
			{
				lua_Number value;
				CHECK_SIZE(*size, 8);
				value = *(double*)*b;
				*size -= 8, *b += 8;
				lua_pushstring(L, node->desc);
				lua_pushnumber(L, value);
				lua_settable(L, -3);
			}
			break;
		case PB_PACKAGE_VALUE_TYPE_STRING:
			{
				uint32_t len;
				CHECK_SIZE(*size, 4);
				len = *(uint32_t*)*b;
				*size -= 4, *b += 4;
				CHECK_SIZE(*size, len);
				lua_pushstring(L, node->desc);
				lua_pushlstring(L, (const char*)*b, len);
				lua_settable(L, -3);
				*size -= len, *b += len;
			}
			break;
		default:
			error_log("unknown value type:%u\n", node->value_type);
			return false;
	}
	return true;
}
bool lua_pb_protocol_decode_package_node(lua_State* L, lua_pb_package_node_t* node, BYTE** b, size_t* size);
bool lua_pb_protocol_decode_package_node_o(lua_State* L, lua_pb_package_node_t* node, BYTE** b, size_t* size)
{
	bool retcode = true;
	size_t n;

	lua_pushstring(L, node->desc);
	lua_newtable(L);
	for (n = 0; n < node->size && retcode; ++n)
	{
		retcode = lua_pb_protocol_decode_package_node(L, node->child[n], b, size);
	}
	lua_settable(L, -3);

	return retcode;
}
bool lua_pb_protocol_decode_package_node_ary(lua_State* L, lua_pb_package_node_t* node, BYTE** b, size_t* size)
{
	bool retcode = true;
	uint32_t ary_size, n, len;

	CHECK_SIZE(*size, 4);
	ary_size = *(uint32_t*)b;
	*size -= 4, *b += 4;

	lua_pushstring(L, node->desc);
	lua_newtable(L);
	for (n = 0; n < ary_size && retcode; ++n)
	{
		/* read `key` type */
		CHECK_SIZE(*size, 1);
		lua_Integer keytype = *(int8_t*)*b;
		--*size, ++*b;

		if (keytype == PB_PACKAGE_VALUE_TYPE_INT)
		{
			CHECK_SIZE(*size, 4);
			lua_Integer key = *(int32_t*)*b;
			*size -= 4, *b += 4;
			lua_pushinteger(L, key);
		}
		else if (keytype == PB_PACKAGE_VALUE_TYPE_STRING)
		{
			CHECK_SIZE(*size, 4);
			len = *(uint32_t*)*b;
			*size -= 4, *b += 4;

			CHECK_SIZE(*size, len);
			lua_pushlstring(L, (const char*)*b, len);
			*size -= len, *b += len;
		}
		else
		{
			error_log("illegue key type:%ld\n", keytype);
			return false;
		}

		lua_newtable(L);
		for (n = 0; n < node->size && retcode; ++n)
		{
			retcode = lua_pb_protocol_decode_package_node(L, node->child[n], b, size);
		}
		lua_settable(L, -3);
	}
	lua_settable(L, -3);

	return retcode;
}
bool lua_pb_protocol_decode_package_node(lua_State* L, lua_pb_package_node_t* node, BYTE** b, size_t* size)
{
	switch(node->node_type)
	{
		case PB_PACKAGE_NODE_TYPE_V:
			return lua_pb_protocol_decode_package_node_v(L, node, b, size);
		case PB_PACKAGE_NODE_TYPE_O:
			return lua_pb_protocol_decode_package_node_o(L, node, b, size);
		case PB_PACKAGE_NODE_TYPE_ARY:
			return lua_pb_protocol_decode_package_node_ary(L, node, b, size);
		default: 
			error_log("error node type:%d\n", node->node_type); break;
	}
	return false;
}
bool lua_pb_protocol_decode_package(lua_State* L, lua_pb_package_t* package, BYTE** b, size_t* size)
{
	bool retcode = true;
	size_t n;

	lua_newtable(L);
	for (n = 0; n < package->root->size && retcode; ++n)
	{
		retcode = lua_pb_protocol_decode_package_node(L, package->root->child[n], b, size);
	}

	return retcode;
}
bool lua_pb_protocol_decode(lua_pb_t* pb, lua_State* L, int cmd, BYTE* b, size_t size)
{
	lua_pb_package_t* package = lua_pb_package_find(pb, cmd);
	if (!package)
	{
 		error_log("error cmd code:%u\n", cmd);
		return false;
	}
	
	if (!lua_pb_protocol_decode_package(L, package, &b, &size))
	{
		error_log("decode package error:%d\n", cmd);
		return false;
	}

	if (size > 0)
	{
		error_log("package remain data:%d, size:%ld\n", cmd, size);
	}

	return true;
}
//------------------------------------------- encode ---------------------------------------------------
bool lua_pb_protocol_encode_package_node_v(lua_State* L, lua_pb_package_node_t* node, BYTE** b, size_t* realsize, size_t size)
{
	lua_pushstring(L, node->desc);
	lua_gettable(L, -2);
	if (lua_isnoneornil(L, -1))
	{
		error_log("encode package found nil value:%s\n", node->desc);
		return false;
	}
	switch(node->value_type)
	{
		case PB_PACKAGE_VALUE_TYPE_CHAR:
			{
				CHECK_BUFSIZE(size, *realsize, 1);
				*(int8_t*)*b = lua_tointeger(L, -1);
				++*realsize, ++*b;
			}
			break;
		case PB_PACKAGE_VALUE_TYPE_SHORT:
			{
				CHECK_BUFSIZE(size, *realsize, 2);
				*(int16_t*)*b = lua_tointeger(L, -1);
				*realsize += 2, *b += 2;
			}
			break;
		case PB_PACKAGE_VALUE_TYPE_INT:
			{
				CHECK_BUFSIZE(size, *realsize, 4);
				*(int32_t*)*b = lua_tointeger(L, -1);
				*realsize += 4, *b += 4;
			}
			break;
		case PB_PACKAGE_VALUE_TYPE_LONG:
			{
				CHECK_BUFSIZE(size, *realsize, 8);
				*(int64_t*)*b = lua_tointeger(L, -1);
				*realsize += 8, *b += 8;
			}
			break;
		case PB_PACKAGE_VALUE_TYPE_BOOL:
			{
				CHECK_BUFSIZE(size, *realsize, 1);				
				*(int8_t*)*b = lua_toboolean(L, -1);
				++*realsize, ++*b;
			}
			break;
		case PB_PACKAGE_VALUE_TYPE_DOUBLE:
			{
				CHECK_BUFSIZE(size, *realsize, 8);
				*(double*)*b = lua_tonumber(L, -1);
				*realsize += 8, *b += 8;
			}
			break;
		case PB_PACKAGE_VALUE_TYPE_STRING:
			{
				size_t len = 0;
				const char* s = lua_tolstring(L, -1, &len);

				CHECK_BUFSIZE(size, *realsize, 4);
				/* don't padding '\0' */
				*(uint32_t*)*b = len;
				*realsize += 4, *b += 4;

				CHECK_BUFSIZE(size, *realsize, len);
				memcpy(*b, s, len);
				*realsize += len, *b += len;
			}
			break;
		default:
			error_log("unknown value type:%u\n", node->value_type);
			return false;
	}
	lua_pop(L, 1);
	return true;
}
bool lua_pb_protocol_encode_package_node(lua_State* L, lua_pb_package_node_t* node, BYTE** b, size_t* realsize, size_t size);
bool lua_pb_protocol_encode_package_node_o(lua_State* L, lua_pb_package_node_t* node, BYTE** b, size_t* realsize, size_t size)
{
	bool retcode = true;
	size_t n;

	lua_pushstring(L, node->desc);
	lua_gettable(L, -2);

	if (!lua_istable(L, -1))
	{
		error_log("stack top not table\n");
		return false;
	}

	for (n = 0; n < node->size && retcode; ++n)
	{
		retcode = lua_pb_protocol_encode_package_node(L, node->child[n], b, realsize, size);
	}

	lua_pop(L, 1);/* remove `table` */
	
	return retcode;
}
bool lua_pb_protocol_encode_package_node_ary(lua_State* L, lua_pb_package_node_t* node, BYTE** b, size_t* realsize, size_t size)
{
	bool retcode = true;
	uint32_t arysize, *arysize_ptr;
	size_t n, len;
	int table_index;
	const char* s = NULL;

	lua_pushstring(L, node->desc);
	lua_gettable(L, -2);

	if (!lua_istable(L, -1))
	{
		error_log("stack top not table:%s\n", node->desc);
		return false;
	}

	CHECK_BUFSIZE(size, *realsize, 4);
	arysize = 0;
	arysize_ptr = (uint32_t*)*b;
	*realsize += 4, *b += 4;

	table_index = lua_gettop(L);

	lua_pushnil(L);
	while (lua_next(L, table_index) != 0)
	{
		/* 'key' is at index -2 and 'value' at index -1, here, `value` is a table */
		if (lua_isnumber(L, -2))
		{
			/* push `key type` */
			CHECK_BUFSIZE(size, *realsize, 1);
			*(int8_t*)*b = PB_PACKAGE_VALUE_TYPE_INT;
			++*realsize, ++*b;

			/* push `key` */
			CHECK_BUFSIZE(size, *realsize, 4);
			*(int32_t*)*b = lua_tointeger(L, -2);
			*realsize += 4, *b += 4;
		}
		else if (lua_isstring(L, -2))
		{
			/* push `key type` */
			CHECK_BUFSIZE(size, *realsize, 1);
			*(int8_t*)*b = PB_PACKAGE_VALUE_TYPE_STRING;
			++*realsize, ++*b;

			/* push `key` */
			len = 0;
			s = lua_tolstring(L, -2, &len);

			CHECK_BUFSIZE(size, *realsize, 4);
			/* don't padding '\0' */
			/* *(uint32_t*)*b = len + 1;//padding '\0' */
			*(uint32_t*)*b = len;
			*realsize += 4, *b += 4;

			CHECK_BUFSIZE(size, *realsize, len);
			memcpy(*b, s, len);
			*realsize += len, *b += len;
		}
		else
		{
			error_log("not support `key` type:%s\n", lua_typename(L, lua_type(L, -2)));
			return false;
		}

		for (n = 0; n < node->size && retcode; ++n)
		{
			retcode = lua_pb_protocol_encode_package_node(L, node->child[n], b, realsize, size);
		}
		++arysize;
		lua_pop(L, 1);/* removes 'value'; keeps 'key' for next iteration */
	}
	*arysize_ptr = arysize;

	lua_pop(L, 1);/* remove `table` */
	
	return retcode;
}
bool lua_pb_protocol_encode_package_node(lua_State* L, lua_pb_package_node_t* node, BYTE** b, size_t* realsize, size_t size)
{
	switch(node->node_type)
	{
		case PB_PACKAGE_NODE_TYPE_V:
			return lua_pb_protocol_encode_package_node_v(L, node, b, realsize, size);
		case PB_PACKAGE_NODE_TYPE_O:
			return lua_pb_protocol_encode_package_node_o(L, node, b, realsize, size);
		case PB_PACKAGE_NODE_TYPE_ARY:
			return lua_pb_protocol_encode_package_node_ary(L, node, b, realsize, size);
		default: 
			error_log("unknown node type:%d\n", node->node_type); break;
	}
	return false;
}
bool lua_pb_protocol_encode_package(lua_State* L, lua_pb_package_t* package, BYTE** b, size_t* realsize, size_t size)
{
	bool retcode = true;
	size_t n;
	for (n = 0; n < package->root->size && retcode; ++n)
	{
		retcode = lua_pb_protocol_encode_package_node(L, package->root->child[n], b, realsize, size);
	}
	return retcode;
}
bool lua_pb_protocol_encode(lua_pb_t* pb, lua_State* L, int cmd, BYTE* b, size_t* size)
{
	if (!lua_istable(L, -1))
	{
		error_log("stack top not table\n");
		return false;
	}

	lua_pb_package_t* package = lua_pb_package_find(pb, cmd);
	if (!package)
	{
 		error_log("error cmd code:%u\n", cmd);
		return false;
	}

	size_t realsize = 0;
	if (!lua_pb_protocol_encode_package(L, package, &b, &realsize, *size))
	{
		error_log("encode package error:%d\n", cmd);
		return false;
	}
	*size = realsize;
	
	return true;
}

