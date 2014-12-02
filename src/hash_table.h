/*
 * \file: hash_table.h
 * \brief: Created by hushouguo at Oct 30 2014 06:57:24
 */
 
#ifndef __HASH_TABLE_H__
#define __HASH_TABLE_H__

typedef long 	hash_key_t;
typedef void *	hash_value_t;

typedef struct hash_node
{
	hash_key_t			key;
	hash_value_t		value;
	struct hash_node*	next;
} hash_node_t;

typedef struct hash_table
{
	size_t				nodesize;
	size_t				elements;
	hash_node_t**		nodes;
} hash_table_t;

hash_table_t* hash_table_create(size_t prealloc_size);
void hash_table_delete(hash_table_t* t);

bool hash_table_insert(hash_table_t* t, hash_key_t key, hash_value_t value);
bool hash_table_remove(hash_table_t* t, hash_key_t key);
hash_value_t hash_table_find(hash_table_t* t, hash_key_t key);
void hash_table_clear(hash_table_t* t);
size_t hash_table_size(hash_table_t* t);
bool hash_table_empty(hash_table_t* t);

uint32_t hash_string(const char* s, size_t size);

#endif
