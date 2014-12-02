/*
 * \file: lua_stack.h
 * \brief: Created by hushouguo at Nov 06 2014 00:56:32
 */
 
#ifndef __LUA_STACK_H__
#define __LUA_STACK_H__

#define LUA_STACK_SIZE						8192
#define LUA_REGISTER_NAMESPACE				"cc"

typedef struct lua_stack
{
	lua_State*								L;
	int										call_lua_count;
} lua_stack_t;

lua_stack_t* lua_stack_create(int stack_size);
void lua_stack_delete(lua_stack_t* stack);

bool lua_stack_execute_string(lua_stack_t* stack, const char* codes);
bool lua_stack_execute_script_file(lua_stack_t* stack, const char* filename);
void lua_stack_clean(lua_stack_t* stack);
bool lua_stack_push_global_function(lua_stack_t* stack, const char* func_name);
bool lua_stack_execute_function(lua_stack_t* stack, int args);
void lua_stack_dump_root_table(lua_stack_t* stack);

void lua_stack_register_namespace(lua_stack_t* stack, const char* name);
void lua_stack_begin_namespace(lua_stack_t* stack, const char* name);
void lua_stack_end_namespace(lua_stack_t* stack);

void lua_stack_register_function(lua_stack_t* stack, const char* f, lua_CFunction routine);
void lua_stack_register_global_function(lua_stack_t* stack, const char* f, lua_CFunction routine);

void lua_stack_register_integer(lua_stack_t* stack, const char* key, int value);
void lua_stack_register_long(lua_stack_t* stack, const char* key, long value);
void lua_stack_register_double(lua_stack_t* stack, const char* key, double value);
void lua_stack_register_string(lua_stack_t* stack, const char* key, const char* value);

void lua_stack_register_standard_functions(lua_stack_t* stack);

#endif
