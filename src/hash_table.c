/*
 * \file: hash_table.c
 * \brief: Created by hushouguo at Oct 30 2014 10:17:28
 */

#include "zebra.h"

void hash_table_insert_node(hash_table_t* t, hash_node_t* node)
{
	size_t hashid = node->key % t->nodesize;
	node->next = t->nodes[hashid];
	t->nodes[hashid] = node;
}

void hash_table_rehash(hash_table_t* t)
{
	size_t oldnodesize = t->nodesize, n;
	hash_node_t** oldnodes = t->nodes;

	t->nodesize *= 2;
	t->nodes = (hash_node_t**)malloc(sizeof(hash_node_t*) * t->nodesize);
	memset(t->nodes, 0, sizeof(hash_node_t*) * t->nodesize);

	for (n = 0; n < oldnodesize; ++n)
	{
		hash_node_t *node = oldnodes[n];
		while (node)
		{
			hash_node_t *next_node = node->next;
			hash_table_insert_node(t, node);
			node = next_node;
		}
	}

	free(oldnodes);
}		

hash_table_t* hash_table_create(size_t prealloc_size)
{
	hash_table_t* t = (hash_table_t*)malloc(sizeof(hash_table_t));
	memset(t, 0, sizeof(hash_table_t));
	t->nodesize = prealloc_size == 0 ? 32 : prealloc_size;
	t->elements = 0;
	t->nodes = (hash_node_t**)malloc(sizeof(hash_node_t*) * t->nodesize);
	memset(t->nodes, 0, sizeof(hash_node_t*) * t->nodesize);
	return t;
}

void hash_table_delete(hash_table_t* t)
{
	if (t)
	{
		size_t n;
		for (n = 0; n < t->nodesize; ++n)
		{
			hash_node_t* node = t->nodes[n];
			while (node)
			{
				hash_node_t* next_node = node->next;
				free(node);
				node = next_node;
			}
		}
		free(t->nodes);
		free(t);
	}
}

bool hash_table_insert(hash_table_t* t, hash_key_t key, hash_value_t value)
{
	size_t hashid = key % t->nodesize;
	hash_node_t* node = NULL;

	if (hash_table_find(t, key)) { return false; }

	node = (hash_node_t*)malloc(sizeof(hash_node_t));
	memset(node, 0, sizeof(hash_node_t));
	node->key = key;
	node->value = value;
	node->next = t->nodes[hashid];
	t->nodes[hashid] = node;

	if (++t->elements > (t->nodesize - t->nodesize/4))
	{
		/* greater than 3/4 */
		hash_table_rehash(t);
	}

	return true;
}

bool hash_table_remove(hash_table_t* t, hash_key_t key)
{
	size_t hashid = key % t->nodesize;

	hash_node_t* node = t->nodes[hashid];
	hash_node_t* prev = NULL;
	while (node && node->key != key) 
	{
		prev = node;
		node = node->next; 
	}

	if (!node)
	{
		/* not exist key */
		return false;
	}

	if (prev)
	{
		prev->next = node->next;
	}
	else
	{
		t->nodes[hashid] = node->next;
	}

	free(node);
	--t->elements;

	return true;
}

hash_value_t hash_table_find(hash_table_t* t, hash_key_t key)
{
	size_t hashid = key % t->nodesize;
	hash_node_t* node = t->nodes[hashid];
	while (node && node->key != key) { node = node->next; }
	return node ? node->value : NULL;
}

void hash_table_clear(hash_table_t* t)
{
	size_t n;
	t->elements = 0;
	for (n = 0; n < t->nodesize; ++n)
	{
		hash_node_t* node = t->nodes[n];
		while (node) 
		{ 
			hash_node_t* next_node = node->next;
			free(node);
			node = next_node;
		}
		t->nodes[n] = NULL;
	}
}

size_t hash_table_size(hash_table_t* t)
{
	return t->elements;
}

bool hash_table_empty(hash_table_t* t)
{
	return t->elements == 0 ? true : false;
}

uint32_t hash_string(const char* s, size_t size)
{
	uint32_t h = 0, g;
	const char* end = s + size;
	while (s < end)
	{
		h = (h << 4) + *s++;
		if ((g = (h & 0xF0000000)))
		{
			h = h ^ (g >> 24);
			h = h ^ g;
		}
	}
	return h;	
}

