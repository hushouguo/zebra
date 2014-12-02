/*
 * \file: lua_xml_parser.c
 * \brief: Created by hushouguo at Nov 06 2014 17:52:21
 */

#include "zebra.h"

void lua_xml_parser(lua_State* L, xmlDocPtr doc, xmlNodePtr node)
{
	xmlAttrPtr cur;
	if (!node) { return; }

	if (node != xml_parser_root_node(doc)) 
	{ 
		lua_pushstring(L, (const char*)node->name); 
	}
	lua_newtable(L);
	for (cur = node->properties; cur; cur = cur->next)
	{
		lua_pushstring(L, (const char*)cur->name);
		lua_pushstring(L, (const char*)xmlGetProp(node, cur->name));
		lua_settable(L, -3);
	}
	
	lua_xml_parser(L, doc, xml_parser_child_node(doc, node, NULL));

	if (node != xml_parser_root_node(doc)) 
	{
		lua_settable(L, -3); 
	}

	lua_xml_parser(L, doc, xml_parser_next_node(doc, node, NULL));
}
bool lua_xml_parser_decode(lua_State* L, const char* file)
{
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

	lua_xml_parser(L, doc, root);

	xml_parser_close(doc);

	return true;
}
const char* lua_xml_parser_encode(lua_State* L)
{
	return "NOT IMPLEMENT";
}

