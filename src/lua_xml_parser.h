/*
 * \file: lua_xml_parser.h
 * \brief: Created by hushouguo at Nov 06 2014 17:51:52
 */
 
#ifndef __LUA_XML_PARSER_H__
#define __LUA_XML_PARSER_H__

bool lua_xml_parser_decode(lua_State* L, const char* file);
const char* lua_xml_parser_encode(lua_State* L);

#endif
