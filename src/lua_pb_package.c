/*
 * \file: lua_pb_package.c
 * \brief: Created by hushouguo at Nov 07 2014 01:27:43
 */

#include "zebra.h"

struct 
{
	PB_PACKAGE_VALUE_TYPE_ENUM		value_type;
	const char*						type_string;
} lua_pb_package_value_type_array[] = 
{
	{PB_PACKAGE_VALUE_TYPE_NONE,	"none"},
	{PB_PACKAGE_VALUE_TYPE_CHAR,	"char"},
	{PB_PACKAGE_VALUE_TYPE_SHORT,	"short"},
	{PB_PACKAGE_VALUE_TYPE_INT,		"int"},
	{PB_PACKAGE_VALUE_TYPE_LONG,	"long"},
	{PB_PACKAGE_VALUE_TYPE_BOOL,	"bool"},
	{PB_PACKAGE_VALUE_TYPE_DOUBLE,	"double"},
	{PB_PACKAGE_VALUE_TYPE_STRING,	"string"},
};

PB_PACKAGE_VALUE_TYPE_ENUM lua_pb_package_parse_valuetype(const char* type_string)
{
	int n;
	for (n = 0; n < sizeof(lua_pb_package_value_type_array)/sizeof(lua_pb_package_value_type_array[0]); ++n)
	{
		if (strcasecmp(lua_pb_package_value_type_array[n].type_string, type_string) == 0)
		{
			return lua_pb_package_value_type_array[n].value_type;
		}
	}
	return PB_PACKAGE_VALUE_TYPE_NONE; 
}

bool lua_pb_package_parse_node(xmlDocPtr doc, xmlNodePtr node, lua_pb_package_node_t* packagenode);
bool lua_pb_package_parse_macro(xmlDocPtr doc, lua_pb_package_node_t* packagenode, const char* macroname)
{
	xmlNodePtr macronode;
	xmlNodePtr root = xml_parser_root_node(doc);
	for (macronode = xml_parser_child_node(doc, root, "macro"); macronode; macronode = xml_parser_next_node(doc, macronode, "macro"))
	{
		const char* desc = xml_parser_node_value_string(doc, macronode, "desc");
		if (desc && strcasecmp(macroname, desc) == 0)
		{
			return lua_pb_package_parse_node(doc, macronode, packagenode);
		}
	}
	return false;
}
bool lua_pb_package_parse_node(xmlDocPtr doc, xmlNodePtr node, lua_pb_package_node_t* packagenode)
{
	bool retcode = true;
	xmlNodePtr currnode;
	for (currnode = xml_parser_child_node(doc, node, NULL); currnode && retcode; currnode = xml_parser_next_node(doc, currnode, NULL))
	{
		if (strcasecmp((char*)currnode->name, "v") == 0)
		{
			PB_PACKAGE_VALUE_TYPE_ENUM valuetype;
			lua_pb_package_node_t* newnode;
			const char* desc = xml_parser_node_value_string(doc, currnode, "desc");
			const char* type = xml_parser_node_value_string(doc, currnode, "type");
			if (!desc || !type)
			{
				error_log("invalid v node, desc:%s, type:%s\n", desc ? desc : "NULL", type ? type : "NULL");
				return false;
			}
			valuetype = lua_pb_package_parse_valuetype(type);
			if (valuetype == PB_PACKAGE_VALUE_TYPE_NONE)
			{
				return false;
			}
			newnode = (lua_pb_package_node_t*)malloc(sizeof(lua_pb_package_node_t));
			newnode->node_type = PB_PACKAGE_NODE_TYPE_V;
			newnode->value_type = valuetype;
			strncpy(newnode->desc, desc, sizeof(newnode->desc));
			newnode->size = 0;
			packagenode->child[packagenode->size++] = newnode;
		}
		else if (strcasecmp((char*)currnode->name, "o") == 0)
		{
			lua_pb_package_node_t* newnode;
			const char* desc = xml_parser_node_value_string(doc, currnode, "desc");
			if (!desc)
			{
				error_log("invalid o node, no found desc\n");
				return false;
			}
			newnode = (lua_pb_package_node_t*)malloc(sizeof(lua_pb_package_node_t));
			newnode->node_type = PB_PACKAGE_NODE_TYPE_O;
			newnode->value_type = PB_PACKAGE_VALUE_TYPE_NONE;
			strncpy(newnode->desc, desc, sizeof(newnode->desc));
			newnode->size = 0;
			packagenode->child[packagenode->size++] = newnode;
			retcode = lua_pb_package_parse_node(doc, currnode, newnode);
		}
		else if (strcasecmp((char*)currnode->name, "ary") == 0)
		{
			lua_pb_package_node_t* newnode;
			const char* desc = xml_parser_node_value_string(doc, currnode, "desc");
			if (!desc)
			{
				error_log("invalid ary node, no found desc\n");
				return false;
			}
			newnode = (lua_pb_package_node_t*)malloc(sizeof(lua_pb_package_node_t));
			newnode->node_type = PB_PACKAGE_NODE_TYPE_ARY;
			newnode->value_type = PB_PACKAGE_VALUE_TYPE_NONE;
			strncpy(newnode->desc, desc, sizeof(newnode->desc));
			newnode->size = 0;
			packagenode->child[packagenode->size++] = newnode;
			retcode = lua_pb_package_parse_node(doc, currnode, newnode);
		}
		else
		{
			if (!lua_pb_package_parse_macro(doc, packagenode, (char*)currnode->name))
			{
				error_log("Unknown node name:%s\n", currnode->name);
				return false;
			}
		}
	}
	return retcode;
}
bool lua_pb_package_parse_opnode(lua_pb_t* pb, xmlDocPtr doc, xmlNodePtr node)
{
	bool retcode = true;
	xmlNodePtr opnode;
	for (opnode = xml_parser_child_node(doc, node, "cmd"); opnode && retcode; opnode = xml_parser_next_node(doc, opnode, "cmd"))
	{
		lua_pb_package_t* package = NULL;
		int id = xml_parser_node_value_int(doc, opnode, "id");
		const char* desc = xml_parser_node_value_string(doc, opnode, "desc");
		if (id == 0 || !desc || strlen(desc) == 0)
		{
			error_log("invalid cmd.id:%u or cmd.desc:\"%s\"\n", id, desc ? desc : "NULL");
			return false;
		}
		package = (lua_pb_package_t*)malloc(sizeof(lua_pb_package_t));
		package->root = (lua_pb_package_node_t*)malloc(sizeof(lua_pb_package_node_t));
		package->root->cmd = id;
		strncpy(package->root->desc, desc, sizeof(package->root->desc));
		package->root->size = 0;
		retcode = lua_pb_package_parse_node(doc, opnode, package->root);
		if (retcode)
		{
			if (!hash_table_insert(pb->pb_package_table, package->root->cmd, package))
			{
				error_log("duplicate cmd:%d,%s\n", package->root->cmd, package->root->desc);
				return false;
			}
		}
	}
	return retcode;
}
bool lua_pb_package_parse_file(lua_pb_t* pb, const char* file)
{
	bool retcode;
	xmlNodePtr root;
	xmlDocPtr doc = xml_parser_open(file);
	if (!doc)
	{
		return false;
	}
	root = xml_parser_root_node(doc);
	if (!root)
	{
		error_log("not found root node:%s\n", file);
		return false;
	}
	retcode = lua_pb_package_parse_opnode(pb, doc, root);
	xml_parser_close(doc);
	return retcode;
}
void lua_pb_package_node_dump(lua_pb_package_node_t* node, const char* prefix)
{
	size_t n;
	debug_log("%sdesc:%s\n", prefix, node->desc);
	for (n = 0; n < node->size; ++n)
	{
		char buffer[96];
		snprintf(buffer, sizeof(buffer), "%s\t", prefix);
		lua_pb_package_node_dump(node->child[n], buffer);
	}
}
void lua_pb_package_dump(lua_pb_t* pb)
{
	size_t n = 0;
	debug_log("lua_pb_package_dump\n");
    for (n = 0; n < pb->pb_package_table->nodesize; ++n)
    {
        hash_node_t* node = pb->pb_package_table->nodes[n];
        while (node)
        {
			lua_pb_package_t* package = (lua_pb_package_t*)node->value;
			size_t i;
			debug_log("cmd:%d\n", package->root->cmd);
			for (i = 0; i < package->root->size; ++i)
			{
				lua_pb_package_node_dump(package->root->child[i], "\t");
			}
			node = node->next;
        }
    }	
}

bool lua_pb_package_init(lua_pb_t* pb, const char* file)
{
	bool retcode = lua_pb_package_parse_file(pb, file);
	/* lua_pb_package_dump(pb); */
	return retcode;
}
void lua_pb_package_destroy_node(lua_pb_package_node_t* node)
{
	size_t n;
	for (n = 0; n < node->size; ++n)
	{
		lua_pb_package_destroy_node(node->child[n]);
	}
	SAFE_FREE(node);
}
void lua_pb_package_destroy(lua_pb_t* pb)
{
	size_t n = 0;
    for (n = 0; n < pb->pb_package_table->nodesize; ++n)
    {
        hash_node_t* node = pb->pb_package_table->nodes[n];
        while (node)
        {
			lua_pb_package_t* package = (lua_pb_package_t*)node->value;
			size_t i;
			for (i = 0; i < package->root->size; ++i)
			{
				lua_pb_package_destroy_node(package->root->child[i]);
			}
			SAFE_FREE(package);
			node = node->next;
        }
    }
	hash_table_clear(pb->pb_package_table);
}
lua_pb_package_t* lua_pb_package_find(lua_pb_t* pb, int cmd)
{
	return (lua_pb_package_t*)hash_table_find(pb->pb_package_table, cmd);
}

