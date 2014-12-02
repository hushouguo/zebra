/*
 * \file: configure.c
 * \brief: Created by hushouguo at Nov 09 2014 03:57:14
 */

#include "zebra.h"

#define DEF_CONF_TABLE_SIZE		960
static hash_table_t* conf_table = NULL;

typedef struct conf_node
{
	char*	key;
	char*	value;
} conf_node_t;

static void make_conf_slot(xmlDocPtr doc, xmlNodePtr node, const char* name)
{
	xmlAttrPtr prop;
	if (!node) { return; }
	for (prop = node->properties; prop; prop = prop->next)
	{
		if (node == xml_parser_root_node(doc))
		{
			char buf[1024];
			snprintf(buf, sizeof(buf), "%s%s.%s", name, (const char*)node->name, (const char*)prop->name);
			conf_set_value(buf, (const char*)xmlGetProp(node, prop->name));
		}
		else
		{
			char buf[1024];
			snprintf(buf, sizeof(buf), "%s.%s.%s", name, (const char*)node->name, (const char*)prop->name);
			conf_set_value(buf, (const char*)xmlGetProp(node, prop->name));
		}
	}

	if (node == xml_parser_root_node(doc))
	{
		make_conf_slot(doc, xml_parser_child_node(doc, node, NULL), name);
	}
	else
	{
		char buf[1024];
		if (strlen(name) > 0)
		{
			snprintf(buf, sizeof(buf), "%s.%s", name, (const char*)node->name);
		}
		else
		{
			snprintf(buf, sizeof(buf), "%s%s", name, (const char*)node->name);
		}
		make_conf_slot(doc, xml_parser_child_node(doc, node, NULL), buf);
	}

	make_conf_slot(doc, xml_parser_next_node(doc, node, NULL), name);
}
bool conf_init(const char* file)
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
	if (conf_table)
	{ 
		hash_table_delete(conf_table); 
	}
	conf_table = hash_table_create(DEF_CONF_TABLE_SIZE);
	make_conf_slot(doc, root, "root");
	xml_parser_close(doc);
	return true;
}

void conf_clean()
{
	/* Todo: free hash value */
	hash_table_delete(conf_table);
	conf_table = NULL;
}

bool conf_set_value(const char* key, const char* value)
{
	uint32_t hash_key = hash_string(key, strlen(key));
	conf_node_t* node = (conf_node_t*)malloc(sizeof(conf_node_t));
	node->key = strdup(key);
	node->value = strdup(value);
	if (!conf_table) { conf_table = hash_table_create(DEF_CONF_TABLE_SIZE); }
	if (!hash_table_insert(conf_table, hash_key, node))
	{
		error_log("duplicate conf key:%s\n", key);
		SAFE_FREE(node->key);
		SAFE_FREE(node->value);
		SAFE_FREE(node);
		return false;
	}
	return true;
}

const char* conf_get_value(const char* key)
{
	conf_node_t* node;
	uint32_t hash_key = hash_string(key, strlen(key));
	if (!conf_table) 
	{ 
		conf_table = hash_table_create(DEF_CONF_TABLE_SIZE);
		return NULL;
	}
	node = (conf_node_t*)hash_table_find(conf_table, hash_key);
	return node ? node->value : NULL;
}

void conf_print()
{
	size_t n, i;
	debug_log("conf:\n");
    for (n = 0; n < conf_table->nodesize; ++n)
    {   
        hash_node_t* node = conf_table->nodes[n];
        while (node)
        {
			conf_node_t* conf_node = (conf_node_t*)node->value;
			debug_log("\t%-s ", conf_node->key);
			for (i = 0; i < 48 - strlen(conf_node->key); ++i) { debug_log("."); }
			debug_log(" %s\n", conf_node->value);
			node = node->next;
        }   
    }	
}

