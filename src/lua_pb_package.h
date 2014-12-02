/*
 * \file: lua_pb_package.h
 * \brief: Created by hushouguo at Nov 07 2014 01:25:01
 */
 
#ifndef __LUA_PB_PACKAGE_H__
#define __LUA_PB_PACKAGE_H__

typedef enum
{
	PB_PACKAGE_NODE_TYPE_NONE						= 0,
	PB_PACKAGE_NODE_TYPE_V							= 1,
	PB_PACKAGE_NODE_TYPE_O							= 2,
	PB_PACKAGE_NODE_TYPE_ARY						= 3,
} PB_PACKAGE_NODE_TYPE_ENUM;

typedef enum
{
	PB_PACKAGE_VALUE_TYPE_NONE						= 0,
	PB_PACKAGE_VALUE_TYPE_CHAR 						= 1,
	PB_PACKAGE_VALUE_TYPE_SHORT						= 2,
	PB_PACKAGE_VALUE_TYPE_INT						= 3,
	PB_PACKAGE_VALUE_TYPE_LONG						= 4,
	PB_PACKAGE_VALUE_TYPE_BOOL						= 5,
	PB_PACKAGE_VALUE_TYPE_DOUBLE					= 6,
	PB_PACKAGE_VALUE_TYPE_STRING					= 7,
} PB_PACKAGE_VALUE_TYPE_ENUM;

typedef enum
{
	PB_PACKAGE_TYPE_MACRO							= 1,
	PB_PACKAGE_TYPE_CS								= 2,
	PB_PACKAGE_TYPE_SC								= 3,
	PB_PACKAGE_TYPE_SS								= 4,
} PB_PACKAGE_TYPE_ENUM;

#define MAX_PB_PACKAGE_NODE_DESC_SIZE				32

typedef struct lua_pb_package_node
{
	union
	{
		struct
		{
			PB_PACKAGE_NODE_TYPE_ENUM				node_type;
			PB_PACKAGE_VALUE_TYPE_ENUM				value_type;
		};
		struct
		{
			int										cmd;
		};
	};
	char											desc[MAX_PB_PACKAGE_NODE_DESC_SIZE];
	size_t											size;
	struct lua_pb_package_node*						child[96];
} lua_pb_package_node_t;

typedef struct lua_pb_package
{
	lua_pb_package_node_t*							root;
} lua_pb_package_t;

struct lua_pb;
bool lua_pb_package_init(struct lua_pb* pb, const char* file);
void lua_pb_package_destroy(struct lua_pb* pb);
lua_pb_package_t* lua_pb_package_find(struct lua_pb* pb, int cmd);

#endif
