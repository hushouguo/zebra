/*
 * \file: record.h
 * \brief: Created by hushouguo at Nov 08 2014 09:07:41
 */
 
#ifndef __RECORD_H__
#define __RECORD_H__

#define DEF_TABLE_SECTION		8
#define MAX_RECORD_SIZE			4*MB

typedef struct mysql_config
{
	const char*	host;
	const char*	user;
	const char*	passwd;
	const char*	db;
	int port;
} mysql_config_t;

typedef struct database_handler
{
	int					id;
	MYSQL				mysql_handler;
	MYSQL_STMT*			stmt;
	mysql_config_t		config;
} database_handler_t;

database_handler_t* database_handler_create(mysql_config_t* config);
bool database_handler_reopen(database_handler_t* handler);
void database_handler_delete(database_handler_t* handler);

bool table_create(database_handler_t* handler, const char* tablename);
bool table_delete(database_handler_t* handler, const char* tablename);

bool record_create(database_handler_t* handler, const char* tablename, int64_t id, const void* data, size_t len);
bool record_delete(database_handler_t* handler, const char* tablename, int64_t id);
bool record_update(database_handler_t* handler, const char* tablename, int64_t id, const void* data, size_t len);
bool record_select(database_handler_t* handler, const char* tablename, int64_t id, void* data, size_t* len);
uint64_t record_insert_id(database_handler_t* handler);

void lua_stack_register_record_functions(lua_stack_t* stack);

#endif
