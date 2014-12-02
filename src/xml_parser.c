/*
 * \file: xml_parser.c
 * \brief: Created by hushouguo at Oct 30 2014 02:00:59
 */

#include "zebra.h"

xmlDocPtr xml_parser_open(const char* xmlfile)
{
	xmlDocPtr doc = xmlParseFile(xmlfile);
	if (!doc)
	{
		xmlErrorPtr p = xmlGetLastError();
		if (p)
		{
			error_log("[%s]:%u, %s\n", p->file, p->line, p->message);
		}
	}
	return doc;	
}

void xml_parser_close(xmlDocPtr doc)
{
	if (doc)
	{
		xmlFreeDoc(doc);
		doc = NULL;
	}	
}

xmlNodePtr xml_parser_root_node(xmlDocPtr doc)
{
	return xmlDocGetRootElement(doc);	
}

xmlNodePtr xml_parser_child_node(xmlDocPtr doc, xmlNodePtr node, const char* nodename)
{
	xmlNodePtr child_node = node->xmlChildrenNode;
	while (child_node)
	{
		if (nodename)
		{
			if ((!xmlStrcmp(child_node->name, (const xmlChar *)nodename)))
			{
				return child_node;
			}
		}
		else
		{
			if (child_node->type == XML_ELEMENT_NODE)
			{
				return child_node;
			}			
		}
		child_node = child_node->next;
	}
	return NULL;
}

xmlNodePtr xml_parser_next_node(xmlDocPtr doc, xmlNodePtr node, const char* nodename)
{
	xmlNodePtr child_node = node->next;
	while (child_node)
	{
		if (nodename)
		{
			if ((!xmlStrcmp(child_node->name, (const xmlChar *)nodename)))
			{
				return child_node;
			}
		}
		else
		{
			if (child_node->type == XML_ELEMENT_NODE)
			{
				return child_node;
			}
		}
		child_node = child_node->next;
	}
	return NULL;
}

int xml_parser_node_value_int(xmlDocPtr doc, xmlNodePtr node, const char* keyname)
{
	xmlChar* value = xmlGetProp(node, (const xmlChar *)keyname);
	return value == (xmlChar*)NULL ? 0 : strtol((char*)value, (char**)NULL, 10);
}

int64_t xml_parser_node_value_int64(xmlDocPtr doc, xmlNodePtr node, const char* keyname)
{
	xmlChar* value = xmlGetProp(node, (const xmlChar *)keyname);
	return value == (xmlChar*)NULL ? 0LL : strtoll((char*)value, (char**)NULL, 10);
}

float xml_parser_node_value_float(xmlDocPtr doc, xmlNodePtr node, const char* keyname)
{
	xmlChar* value = xmlGetProp(node, (const xmlChar *)keyname);
	return value == (xmlChar*)NULL ? 0.f : strtod((char*)value, (char**)NULL);
}

bool xml_parser_node_value_bool(xmlDocPtr doc, xmlNodePtr node, const char* keyname)
{
	xmlChar* value = xmlGetProp(node, (const xmlChar *)keyname);
	if (!strcmp((const char*)value, "true")) { return true; }
	return false;
}

const char *xml_parser_node_value_string(xmlDocPtr doc, xmlNodePtr node, const char* keyname)
{
	xmlChar* value = xmlGetProp(node, (const xmlChar *)keyname);
	return (const char*)value;
}

