/*
 * \file: xml_parser.h
 * \brief: Created by hushouguo at Oct 30 2014 01:53:00
 */
 
#ifndef __XMLPARSER_H__
#define __XMLPARSER_H__

xmlDocPtr xml_parser_open(const char* xmlfile);
void xml_parser_close(xmlDocPtr doc);

xmlNodePtr xml_parser_root_node(xmlDocPtr doc);
xmlNodePtr xml_parser_child_node(xmlDocPtr doc, xmlNodePtr node, const char* nodename);
xmlNodePtr xml_parser_next_node(xmlDocPtr doc, xmlNodePtr node, const char* nodename);
int xml_parser_node_value_int(xmlDocPtr doc, xmlNodePtr node, const char* keyname);
int64_t xml_parser_node_value_int64(xmlDocPtr doc, xmlNodePtr node, const char* keyname);
float xml_parser_node_value_float(xmlDocPtr doc, xmlNodePtr node, const char* keyname);
bool xml_parser_node_value_bool(xmlDocPtr doc, xmlNodePtr node, const char* keyname);
const char* xml_parser_node_value_string(xmlDocPtr doc, xmlNodePtr node, const char* keyname);

#endif
