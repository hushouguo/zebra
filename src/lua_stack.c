/*
 * \file: lua_stack.c
 * \brief: Created by hushouguo at Nov 06 2014 01:02:14
 */

#include "zebra.h"

#define LUA_REGISTER(L, F, ROUTINE) \
	lua_pushstring(L, F);\
	lua_pushcfunction(L, ROUTINE);\
	lua_rawset(L, -3);

#define DEF_TIMESTAMP_FORMAT	"%y/%02m/%02d %02H:%02M:%02S"

static bool lua_print_log(lua_State* L)
{
	if (lua_istable(L, -1))
	{
		lua_debug_log("[table]\n");
	}
	else if (lua_isnone(L, -1))
	{
		lua_debug_log("[none]\n");
	}
	else if (lua_isnil(L, -1))
	{
		lua_debug_log("[nil]\n");
	}
	else if (lua_isboolean(L, -1))
	{
		if (lua_toboolean(L, -1) != 0)
		{
			lua_debug_log("[true]\n");
		}
		else
		{
			lua_debug_log("[false]\n");
		}
	}
	else if (lua_isfunction(L, -1))
	{
		lua_debug_log("[function]\n");
	}
	else if (lua_islightuserdata(L, -1))
	{
		lua_debug_log("[lightuserdata]\n");
	}
	else if (lua_isthread(L, -1))
	{
		lua_debug_log("[thread]\n");
	}
	else 
	{ 
		return false; 
	}
	return true;
}
/* void debug_log(o) */
static int cc_debug_log(lua_State* L)
{
	if (!lua_print_log(L))
	{
		const char* str = lua_tostring(L, -1);
		if (str)
		{
			lua_debug_log("%s", str);
		}
		else
		{
			lua_debug_log("%s", lua_typename(L, lua_type(L, -1)));
		}
	}
	return 0;
}
/* void alarm_log(o) */
static int cc_alarm_log(lua_State* L)
{
	if (!lua_print_log(L))
	{
		const char* str = lua_tostring(L, -1);
		if (str)
		{
			lua_alarm_log("%s", str);
		}
		else
		{
			lua_alarm_log("%s", lua_typename(L, lua_type(L, -1)));
		}
	}
	return 0;
}
/* void error_log(o) */
static int cc_error_log(lua_State* L)
{
	if (!lua_print_log(L))
	{
		const char* str = lua_tostring(L, -1);
		if (str)
		{
			lua_error_log("%s", str);
		}
		else
		{
			lua_error_log("%s", lua_typename(L, lua_type(L, -1)));
		}
	}
	return 0;
}

/* int now() */
static int cc_now(lua_State* L)
{
	time_t now = time(0);
	lua_pushinteger(L, now);
	return 1;
}
/* void sleep(seconds) */
static int cc_sleep(lua_State* L)
{
	int seconds;
	int args = lua_gettop(L);
	if (args < 1)
	{
		error_log("`%s` parameter lack:%d\n", __FUNCTION__, args);
		return 0;
	}

	if (!lua_isnumber(L, -1))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -args)));
		return 0;
	}

	seconds = lua_tointeger(L, -1);

	sleep(seconds);

	return 0;
}
/* void usleep(microseconds) */
static int cc_usleep(lua_State* L)
{
	int microseconds;
	int args = lua_gettop(L);
	if (args < 1)
	{
		error_log("`%s` parameter lack:%d\n", __FUNCTION__, args);
		return 0;
	}

	if (!lua_isnumber(L, -1))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -args)));
		return 0;
	}

	microseconds = lua_tointeger(L, -1);

	usleep(microseconds);

	return 0;
}
/* int,int clocktime() */
static int cc_clocktime(lua_State* L)
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	lua_pushinteger(L, ts.tv_sec);
	lua_pushinteger(L, ts.tv_nsec);
	return 2;
}
/* string timestamp(), timestamp(int), timestamp(string), timestamp(int, string) */
static int cc_timestamp(lua_State* L)
{
	const char* s = NULL;
	int args = lua_gettop(L);
	if (args == 0)
	{
		s = timestamp(time(0), DEF_TIMESTAMP_FORMAT);
	}
	else if (args == 1)
	{
		if (lua_isnumber(L, -1))
		{
			int t = lua_tointeger(L, -1);
			s = timestamp(t, DEF_TIMESTAMP_FORMAT);
		}
		else if (lua_isstring(L, -1))
		{
			const char* format = lua_tostring(L, -1);
			s = timestamp(time(0), format);
		}
		else
		{
			error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -1)));
			return 0;
		}
	}
	else if (args >= 2)
	{
		int t;
		const char* format;
		if (!lua_isnumber(L, -args))
		{
			error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -args)));
			return 0;
		}
		if (!lua_isstring(L, -(args-1)))
		{
			error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -(args-1))));
			return 0;
		}
		t = lua_tointeger(L, -args);
		format = lua_tostring(L, -(args-1));
		s = timestamp(t, format);
	}

	lua_pushstring(L, s);

	return 1;
}
/* int year(), year(int time) */
static int cc_year(lua_State* L)
{
	struct tm m;
	time_t t = 0;
	int args = lua_gettop(L);
	if (args == 0)
	{
		t = time(0);
	}
	else
	{
		if (!lua_isnumber(L, -args))
		{
			error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -args)));
			return 0;
		}
		t = lua_tointeger(L, -args);
	}
	localtime_r(&t, &m);
	lua_pushinteger(L, m.tm_year+1900);
	return 1;
}
/* int month(), month(int time) */
static int cc_month(lua_State* L)
{
	struct tm m;
	time_t t = 0;
	int args = lua_gettop(L);
	if (args == 0)
	{
		t = time(0);
	}
	else
	{
		if (!lua_isnumber(L, -args))
		{
			error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -args)));
			return 0;
		}
		t = lua_tointeger(L, -args);
	}
	localtime_r(&t, &m);
	lua_pushinteger(L, m.tm_mon+1);
	return 1;
}
/* int day(), day(int time) */
static int cc_day(lua_State* L)
{
	struct tm m;
	time_t t = 0;
	int args = lua_gettop(L);
	if (args == 0)
	{
		t = time(0);
	}
	else
	{
		if (!lua_isnumber(L, -args))
		{
			error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -args)));
			return 0;
		}
		t = lua_tointeger(L, -args);
	}
	localtime_r(&t, &m);
	lua_pushinteger(L, m.tm_mday);
	return 1;
}
/* int hour(), hour(int time) */
static int cc_hour(lua_State* L)
{
	struct tm m;
	time_t t = 0;
	int args = lua_gettop(L);
	if (args == 0)
	{
		t = time(0);
	}
	else
	{
		if (!lua_isnumber(L, -args))
		{
			error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -args)));
			return 0;
		}
		t = lua_tointeger(L, -args);
	}
	localtime_r(&t, &m);
	lua_pushinteger(L, m.tm_hour);
	return 1;
}
/* int minute(), minute(int time) */
static int cc_minute(lua_State* L)
{
	struct tm m;
	time_t t = 0;
	int args = lua_gettop(L);
	if (args == 0)
	{
		t = time(0);
	}
	else
	{
		if (!lua_isnumber(L, -args))
		{
			error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -args)));
			return 0;
		}
		t = lua_tointeger(L, -args);
	}
	localtime_r(&t, &m);
	lua_pushinteger(L, m.tm_min);
	return 1;
}
/* int second(), second(int time) */
static int cc_second(lua_State* L)
{
	struct tm m;
	time_t t = 0;
	int args = lua_gettop(L);
	if (args == 0)
	{
		t = time(0);
	}
	else
	{
		if (!lua_isnumber(L, -args))
		{
			error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -args)));
			return 0;
		}
		t = lua_tointeger(L, -args);
	}
	localtime_r(&t, &m);
	lua_pushinteger(L, m.tm_sec);
	return 1;
}
/* int rand(), rand(int) */
static int cc_rand(lua_State* L)
{
	int seed = 0;
	int args = lua_gettop(L);
	if (args == 0)
	{
		seed = time(0);
	}
	else
	{
		if (!lua_isnumber(L, -args))
		{
			error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -args)));
			return 0;
		}
		seed = lua_tointeger(L, -args);
	}
	lua_pushinteger(L, rand_r((uint32_t*)&seed));
	return 1;
}
/* int random_between(int min, int max) */
static int cc_random_between(lua_State* L)
{
	int min, max;
	int args = lua_gettop(L);
	if (args < 2)
	{
		error_log("`%s` parameter lack:%d\n", __FUNCTION__, args);
		return 0;
	}

	if (!lua_isnumber(L, -args))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -args)));
		return 0;
	}

	if (!lua_isnumber(L, -(args-1)))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -(args-1))));
		return 0;
	}

	min = lua_tointeger(L, -args);
	max = lua_tointeger(L, -(args-1));

	lua_pushinteger(L, random_between(min, max));

	return 1;
}
/* void halt() */
static int cc_halt(lua_State* L)
{
	halt();
	return 0;
}
/* bool daemon() */
static int cc_daemon(lua_State* L)
{
	lua_pushboolean(L, shard->daemon ? 1 : 0);
	return 1;
}
/* int qsize() */
static int cc_qsize(lua_State* L)
{
	worker_handler_t* handler = find_handler_by_stack(L);
	lua_pushinteger(L, msg_queue_size(handler->q));
	return 1;
}
/* int sid() */
static int cc_sid(lua_State* L)
{
	lua_pushinteger(L, getpid());
	return 1;
}
/* void dup(oldfd, newfd) */
static int cc_dup(lua_State* L)
{
	int oldfd, newfd;
	int args = lua_gettop(L);
	if (args < 2)
	{
		error_log("`%s` parameter lack:%d\n", __FUNCTION__, args);
		return 0;
	}

	if (!lua_isnumber(L, -args))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -args)));
		return 0;
	}

	if (!lua_isnumber(L, -(args-1)))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -(args-1))));
		return 0;
	}

	oldfd = lua_tointeger(L, -args);
	newfd = lua_tointeger(L, -(args-1));

	dup2(oldfd, newfd);
	return 0;
}
/* int hash_string(string) */
static int cc_hash_string(lua_State* L)
{
	size_t len;
	const char* s;
	int args = lua_gettop(L);
	if (args < 1)
	{
		error_log("`%s` parameter lack:%d\n", __FUNCTION__, args);
		return 0;
	}

	if (!lua_isstring(L, -args))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -args)));
		return 0;
	}

	len = 0;
	s = lua_tolstring(L, -args, &len);

	lua_pushinteger(L, hash_string(s, len));

	return 1;
}
/* string configure(string) */
static int cc_configure(lua_State* L)
{
	const char *key, *value;
	int args = lua_gettop(L);
	if (args < 1)
	{
		error_log("`%s` parameter lack:%d\n", __FUNCTION__, args);
		return 0;
	}

	if (!lua_isstring(L, -args))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -args)));
		return 0;
	}

	key = lua_tostring(L, -args);
	value = conf_get_value(key);
	if (!value)
	{
		lua_pushnil(L);
	}
	else
	{
		lua_pushstring(L, value);
	}

	return 1;
}
#if 0
/* string encrypt(string) */
static int cc_encrypt(lua_State* L)
{
	size_t len;
	const char* s;
	/*char buf[MAX_NETWORK_PACKAGE_SIZE];*/
	int args = lua_gettop(L);
	if (args < 1)
	{
		error_log("`%s` parameter lack:%d\n", __FUNCTION__, args);
		return 0;
	}

	if (!lua_isstring(L, -args))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -args)));
		return 0;
	}

	len = 0;
	s = lua_tolstring(L, -args, &len);
	/*memcpy(buf, s, len);*/
	/*const char* value = (const char*)encrylight(buf, len);*/
	encrylight(s, len);
	lua_pushlstring(L, s, len);

	return 1;
}
/* string decrypt(string) */
static int cc_decrypt(lua_State* L)
{
	int args = lua_gettop(L);
	if (args < 1)
	{
		error_log("cc_decrypt parameter lack:%d\n", args);
		return 0;
	}

	if (!lua_isstring(L, -args))
	{
		error_log("cc_decrypt parameter error:%s\n", lua_typename(L, lua_type(L, -args)));
		return 0;
	}

	size_t len = 0;
	const char* s = lua_tolstring(L, -args, &len);
	char buf[MAX_NETWORK_PACKAGE_SIZE];
	memcpy(buf, s, len);

	const char* value = (const char*)decrylight(buf, len);
	lua_pushlstring(L, value, len);

	return 1;
}
#endif
/* o xml_decode(string) */
static int cc_xml_decode(lua_State* L)
{
	size_t len;
	const char* file;
	int args = lua_gettop(L);
	if (args < 1)
	{
		error_log("`%s` parameter lack:%d\n", __FUNCTION__, args);
		return 0;
	}

	if (!lua_isstring(L, -args))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -args)));
		return 0;
	}

	len = 0;
	file = lua_tolstring(L, -args, &len);

	if (!lua_xml_parser_decode(L, file))
	{
		error_log("xml decode error:%s\n", file);
		return 0;
	}

	return 1; 
}
/* string xml_encode(o) */
static int cc_xml_encode(lua_State* L)
{
	return 0;
}
/* o json_decode(string) */
static int cc_json_decode(lua_State* L)
{
	size_t len;
	const char* jsonstr;
	int args = lua_gettop(L);
	if (args < 1)
	{
		error_log("`%s` parameter lack:%d\n", __FUNCTION__, args);
		return 0;
	}

	if (!lua_isstring(L, -args))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -args)));
		return 0;
	}

	len = 0;
	jsonstr = lua_tolstring(L, -args, &len);

	/* debug_log("jsonstr:%s, len:%ld\n", jsonstr, len); */

	if (!lua_json_parser_decode(L, jsonstr, len))
	{
		error_log("json decode error:%s\n", jsonstr);
		return 0;
	}

	return 1;
}
/* string json_encode(o) */
static int cc_json_encode(lua_State* L)
{
	const char* s;
	int args = lua_gettop(L);
	if (args < 1)
	{
		error_log("`%s` parameter lack:%d\n", __FUNCTION__, args);
		return 0;
	}

	if (!lua_istable(L, -args))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -args)));
		return 0;
	}

	s = lua_json_parser_encode(L);
	if (!s)
	{
		lua_pushnil(L);
	}
	else
	{
		lua_pushstring(L, s);
	}

	return 1;
}

lua_stack_t* lua_stack_create(int stack_size)
{
	lua_stack_t* stack = (lua_stack_t*)malloc(sizeof(lua_stack_t));
	memset(stack, 0, sizeof(lua_stack_t));
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);
	lua_checkstack(L, stack_size/*LUA_STACK_SIZE*/);
	stack->L = L;
	stack->call_lua_count = 0;
	return stack;
}

void lua_stack_delete(lua_stack_t* stack)
{
	if (stack)
	{
		lua_State* L = stack->L;
		lua_close(L);
		SAFE_FREE(stack);
	}
}

bool lua_stack_execute_string(lua_stack_t* stack, const char* codes)
{
	lua_State* L = stack->L;
	luaL_loadstring(L, codes);
	return lua_stack_execute_function(stack, 0);
}

bool lua_stack_execute_script_file(lua_stack_t* stack, const char* filename)
{
	char buf[960];
	snprintf(buf, sizeof(buf), "require \"%s\"", filename);
	return lua_stack_execute_string(stack, buf);
}

void lua_stack_clean(lua_stack_t* stack)
{
	lua_State* L = stack->L;
	lua_settop(L, 0);	
}

bool lua_stack_push_global_function(lua_stack_t* stack, const char* func_name)
{
	lua_State* L = stack->L;
	lua_getglobal(L, func_name);
	if (!lua_isfunction(L, -1))
	{
		error_log("name `%s` does not represent a Lua function.\n", func_name);
		lua_pop(L, 1);
		return false;
	}
	return true;	
}

bool lua_stack_execute_function(lua_stack_t* stack, int args)
{
	lua_State* L = stack->L;
	int traceback = 0, error;
	int func_idx = -(args + 1);
	if (!lua_isfunction(L, func_idx))
	{
		error_log("value at stack [%d] is not function\n", func_idx);
		lua_pop(L, args + 1);/* remove function and args */
		return false;
	}

	lua_getglobal(L, "__G_TRACKBACK__"); 	/* L: ...func arg1 arg2 ... G */
	if (!lua_isfunction(L, -1))
	{
		lua_pop(L, 1);						/* L: ...func arg1 arg2 ... */
	}
	else
	{
		lua_insert(L, func_idx - 1);		/* L: ... G func arg1 arg2 ... */
		traceback = func_idx - 1;
	}

	++stack->call_lua_count;
	error = lua_pcall(L, args, 1, traceback);
	--stack->call_lua_count;

	if (error)
	{
		if (traceback == 0)
		{
			error_log("%s\n", lua_tostring(L, -1));/* ... error */
			lua_pop(L, 1); /* remove error message from stack */
		}
		else
		{
			lua_pop(L, 2); /* remove __G_TRACKBACK__ and error message from stack */
		}
		return false;
	}

	/* get return value */
	int ret = 0;
	if (lua_isnumber(L, -1))
	{
		ret = (int)lua_tointeger(L, -1);
	}
	else if (lua_isboolean(L, -1))
	{
		ret = (int)lua_toboolean(L, -1);
	}

	lua_pop(L, 1); /* remove return value from stack, L: ... [G] */

	if (traceback)
	{
		lua_pop(L, 1); /* remove __G_TRACKBACK__ from stack, L: ... */
	}

	return true;
}

static const char* parse_value(lua_State* L, int idx)
{
	if (lua_istable(L, idx))
	{
		return "[table]";
	}
	else if (lua_isnone(L, idx))
	{
		return "[none]";
	}
	else if (lua_isnil(L, idx))
	{
		return "[nil]";
	}
	else if (lua_isboolean(L, idx))
	{
		if (lua_toboolean(L, idx) != 0)
		{
			return "[true]";
		}
		else
		{
			return "[false]";
		}
	}
	else if (lua_isfunction(L, idx))
	{
		return "[function]";
	}
	else if (lua_islightuserdata(L, idx))
	{
		return "[lightuserdata]";
	}
	else if (lua_isthread(L, idx))
	{
		return "[thread]";
	}
	else
	{
		const char* str = lua_tostring(L, idx);
		if (str)
		{
			return str;
		}
	}
	return lua_typename(L, lua_type(L, idx));
}

static void dump_lua_table(lua_State* L, int idx, const char* prefix)
{
	lua_pushnil(L);
	while(lua_next(L, idx))
	{
		/* -2 : key, -1 : value */
		lua_pushvalue(L, -2);
		const char* key = lua_tostring(L, -1);
		const char* value = parse_value(L, -2);
		lua_pop(L, 1);

		debug_log("%s%15s: %s\n", prefix, key, value);

		if (lua_istable(L, -1) && strcasecmp(key, "cc") == 0)
		{
			char buffer[960];
			snprintf(buffer, sizeof(buffer), "%s\t", prefix);
			debug_log("%15s{\n", prefix);
			dump_lua_table(L, lua_gettop(L), buffer);
			debug_log("%15s}\n", prefix);
		}
		
		lua_pop(L, 1);/* removes 'value'; keeps 'key' for next iteration */
	}
}

void lua_stack_dump_root_table(lua_stack_t* stack)
{
	lua_State* L = stack->L;
	lua_getglobal(L, "_G");
	debug_log("dump root table\n");
	debug_log("{\n");
	dump_lua_table(L, lua_gettop(L), "\t");
	debug_log("}\n");
	lua_pop(L, 1);/* remove `table` */	
}

void lua_stack_register_namespace(lua_stack_t* stack, const char* name)
{
	lua_State* L = stack->L;
	lua_getglobal(L, "_G");
    lua_pushstring(L, name);
    lua_newtable(L);
	lua_rawset(L, -3);
    lua_pop(L, 1);	
}

void lua_stack_begin_namespace(lua_stack_t* stack, const char* name)
{
	lua_State* L = stack->L;
	lua_getglobal(L, "_G");
	lua_pushstring(L, name);
	lua_rawget(L, -2);	
}

void lua_stack_end_namespace(lua_stack_t* stack)
{
	lua_State* L = stack->L;
	lua_pop(L, 2);	
}

void lua_stack_register_function(lua_stack_t* stack, const char* f, lua_CFunction routine)
{
	lua_State* L = stack->L;
	lua_stack_begin_namespace(stack, LUA_REGISTER_NAMESPACE);
	lua_pushstring(L, f);
	lua_pushcfunction(L, routine);
	lua_rawset(L, -3);
	lua_stack_end_namespace(stack);
}

void lua_stack_register_global_function(lua_stack_t* stack, const char* f, lua_CFunction routine)
{
	lua_State* L = stack->L;
	lua_getglobal(L, "_G");
	lua_pushstring(L, f);
	lua_pushcfunction(L, routine);
	lua_rawset(L, -3);
    lua_pop(L, 1);	
}

void lua_stack_register_integer(lua_stack_t* stack, const char* key, int value)
{
	lua_State* L = stack->L;
	lua_stack_begin_namespace(stack, LUA_REGISTER_NAMESPACE);
	lua_pushstring(L, key);
	lua_pushinteger(L, value);
	lua_settable(L, -3);
	lua_stack_end_namespace(stack);
}

void lua_stack_register_long(lua_stack_t* stack, const char* key, long value)
{
	lua_State* L = stack->L;
	lua_stack_begin_namespace(stack, LUA_REGISTER_NAMESPACE);
	lua_pushstring(L, key);
	lua_pushnumber(L, value);
	lua_settable(L, -3);
	lua_stack_end_namespace(stack);
}

void lua_stack_register_double(lua_stack_t* stack, const char* key, double value)
{
	lua_State* L = stack->L;
	lua_stack_begin_namespace(stack, LUA_REGISTER_NAMESPACE);
	lua_pushstring(L, key);
	lua_pushnumber(L, value);
	lua_settable(L, -3);
	lua_stack_end_namespace(stack);
}

void lua_stack_register_string(lua_stack_t* stack, const char* key, const char* value)
{
	lua_State* L = stack->L;
	lua_stack_begin_namespace(stack, LUA_REGISTER_NAMESPACE);
	lua_pushstring(L, key);
	lua_pushstring(L, value);
	lua_settable(L, -3);
	lua_stack_end_namespace(stack);
}

void lua_stack_register_standard_functions(lua_stack_t* stack)
{
	lua_State* L = stack->L;

	lua_stack_begin_namespace(stack, LUA_REGISTER_NAMESPACE);

	/* logger */
	LUA_REGISTER(L, "debug_log", cc_debug_log);					/* void debug_log(s) */
	LUA_REGISTER(L, "alarm_log", cc_alarm_log);					/* void alarm_log(s) */	
	LUA_REGISTER(L, "error_log", cc_error_log);					/* void error_log(s) */

	/* time */
	LUA_REGISTER(L, "now", cc_now);								/* int now() */
	LUA_REGISTER(L, "sleep", cc_sleep);							/* void sleep(seconds) */
	LUA_REGISTER(L, "usleep", cc_usleep);						/* void usleep(microseconds) */
	LUA_REGISTER(L, "clocktime", cc_clocktime);					/* int,int clocktime() */
	LUA_REGISTER(L, "timestamp", cc_timestamp);					/* string timestamp(), timestamp(int), timestamp(int, "") */
	LUA_REGISTER(L, "year", cc_year);							/* int year(), year(int) */
	LUA_REGISTER(L, "month", cc_month);							/* int month(), month(int) */
	LUA_REGISTER(L, "day", cc_day);								/* int day(), day(int) */
	LUA_REGISTER(L, "hour", cc_hour);							/* int hour(), hour(int) */
	LUA_REGISTER(L, "minute", cc_minute);						/* int minute(), minute(int) */
	LUA_REGISTER(L, "second", cc_second);						/* int second(), second(int) */

	/* random */
	LUA_REGISTER(L, "rand", cc_rand);							/* int rand(), rand(int) */
	LUA_REGISTER(L, "random_between", cc_random_between);		/* int random_between(int, int) */

	/* xml */
	LUA_REGISTER(L, "xml_decode", cc_xml_decode);				/* o xml_decode(string) */
	LUA_REGISTER(L, "xml_encode", cc_xml_encode);				/* string xml_encode(o) */

	/* json */
	LUA_REGISTER(L, "json_decode", cc_json_decode);				/* o json_decode(string) */
	LUA_REGISTER(L, "json_encode", cc_json_encode);				/* string json_encode(o) */

	/* curl */

	/* encrypt/decrypt */
	//LUA_REGISTER(L, "encrypt", cc_encrypt);						/* string encrypt(string) */
	//LUA_REGISTER(L, "decrypt", cc_decrypt);						/* string decrypt(string) */

	/* helper */
	LUA_REGISTER(L, "halt", cc_halt);							/* void halt() */
	LUA_REGISTER(L, "hash_string", cc_hash_string);				/* int hash_string(string) */
	LUA_REGISTER(L, "dup", cc_dup);								/* void dup(oldfd, newfd) */
	LUA_REGISTER(L, "daemon", cc_daemon);						/* bool daemon() */
	LUA_REGISTER(L, "qsize", cc_qsize);							/* int qsize() */
	LUA_REGISTER(L, "sid", cc_sid);								/* int sid() */

	/* configure */
	LUA_REGISTER(L, "configure", cc_configure);					/* string configure(string) */

	lua_stack_end_namespace(stack);
}

