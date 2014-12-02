/*
 * \file: record.c
 * \brief: Created by hushouguo at Nov 08 2014 11:04:55
 */

#include "zebra.h"

static bool database_prepare(database_handler_t* handler, const char* sql)
{
	if (mysql_stmt_prepare(handler->stmt, sql, strlen(sql)))
	{
		if (mysql_stmt_errno(handler->stmt) == CR_SERVER_GONE_ERROR)
		{
			debug_log("mysql server gone, try to reopen mysql\n");
			if (!database_handler_reopen(handler))
			{
				error_log("reopen failure:%d,%s\n", mysql_stmt_errno(handler->stmt), mysql_stmt_error(handler->stmt));
				return false;
			}
			debug_log("reopen success\n");
			if (mysql_stmt_prepare(handler->stmt, sql, strlen(sql)))
			{
				error_log("mysql_stmt_prepare error:%d,%s\n", mysql_stmt_errno(handler->stmt), mysql_stmt_error(handler->stmt));
				return false;
			}
			return true;
		}
		error_log("mysql_stmt_prepare error:%d,%s, stmt:%s\n", mysql_stmt_errno(handler->stmt), mysql_stmt_error(handler->stmt), sql);
		return false;
	}
	/*uint32_t count = mysql_stmt_param_count(_stmt);*/
	/*debug_log("STATEMENT:%s, total parameters:%u\n", statement, count);*/
	return true;
}

static bool database_bind_param(database_handler_t* handler, MYSQL_BIND* b)
{
	if (mysql_stmt_bind_param(handler->stmt, b))
	{
		if (mysql_stmt_errno(handler->stmt) == CR_SERVER_GONE_ERROR)
		{
			debug_log("try to reopen mysql\n");
			if (!database_handler_reopen(handler))
			{
				error_log("reopen failure:%d,%s\n", mysql_stmt_errno(handler->stmt), mysql_stmt_error(handler->stmt));
				return false;
			}
			debug_log("reopen success\n");
			if (mysql_stmt_bind_param(handler->stmt, b))
			{
				error_log("mysql_stmt_bind_param error:%d,%s\n", mysql_stmt_errno(handler->stmt), mysql_stmt_error(handler->stmt));
				return false;
			}
			return true;
		}
		error_log("mysql_stmt_bind_param error:%d,%s\n", mysql_stmt_errno(handler->stmt), mysql_stmt_error(handler->stmt));
		return false;
	}
	return true;
}

static bool database_bind_result(database_handler_t* handler, MYSQL_BIND* b)
{
	if (mysql_stmt_bind_result(handler->stmt, b))
	{
		if (mysql_stmt_errno(handler->stmt) == CR_SERVER_GONE_ERROR)
		{
			debug_log("try to reopen mysql\n");
			if (!database_handler_reopen(handler))
			{
				error_log("reopen failure:%d,%s\n", mysql_stmt_errno(handler->stmt), mysql_stmt_error(handler->stmt));
				return false;
			}
			debug_log("reopen success\n");
			if (mysql_stmt_bind_result(handler->stmt, b))
			{
				error_log("mysql_stmt_bind_result error:%d,%s\n", mysql_stmt_errno(handler->stmt), mysql_stmt_error(handler->stmt));
				return false;
			}
			return true;
		}
		error_log("mysql_stmt_bind_result error:%d,%s\n", mysql_stmt_errno(handler->stmt), mysql_stmt_error(handler->stmt));
		return false;
	}
	if (mysql_stmt_store_result(handler->stmt))
	{
		if (mysql_stmt_errno(handler->stmt) == CR_SERVER_GONE_ERROR)
		{
			debug_log("try to reopen mysql\n");
			if (!database_handler_reopen(handler))
			{
				error_log("reopen failure:%d,%s\n", mysql_stmt_errno(handler->stmt), mysql_stmt_error(handler->stmt));
				return false;
			}
			debug_log("reopen success\n");
			if (mysql_stmt_store_result(handler->stmt))
			{
				error_log("mysql_stmt_store_result error:%d,%s\n", mysql_stmt_errno(handler->stmt), mysql_stmt_error(handler->stmt));
				return false;
			}
			return true;
		}
		error_log("mysql_stmt_store_result error:%d,%s\n", mysql_stmt_errno(handler->stmt), mysql_stmt_error(handler->stmt));
		return false;
	}
	return true;
}

static bool database_exec(database_handler_t* handler)
{
	if (mysql_stmt_execute(handler->stmt))
	{
		if (mysql_stmt_errno(handler->stmt) == CR_SERVER_GONE_ERROR)
		{
			debug_log("try to reopen mysql\n");
			if (!database_handler_reopen(handler))
			{
				error_log("reopen failure:%d,%s\n", mysql_stmt_errno(handler->stmt), mysql_stmt_error(handler->stmt));
				return false;
			}
			debug_log("reopen success\n");
			if (mysql_stmt_execute(handler->stmt))
			{
				error_log("mysql_stmt_execute error:%d,%s\n", mysql_stmt_errno(handler->stmt), mysql_stmt_error(handler->stmt));
				return false;
			}
			return true;
		}
		error_log("mysql_stmt_execute error:%d,%s\n", mysql_stmt_errno(handler->stmt), mysql_stmt_error(handler->stmt));
		return false;
	}
	/*uint32_t affected = mysql_stmt_affected_rows(handler->stmt);*/
	/*debug_log("This execute affected rows:%u\n", affected);*/
	return true;
}

static bool database_fetch(database_handler_t* handler)
{
	if (mysql_stmt_fetch(handler->stmt))
	{
		if (mysql_stmt_errno(handler->stmt) == CR_SERVER_GONE_ERROR)
		{
			debug_log("try to reopen mysql\n");
			if (!database_handler_reopen(handler))
			{
				error_log("reopen failure:%d,%s\n", mysql_stmt_errno(handler->stmt), mysql_stmt_error(handler->stmt));
				return false;
			}
			debug_log("reopen success\n");
			if (mysql_stmt_fetch(handler->stmt))
			{
				/*error_log("mysql_stmt_prepare error:%d,%s\n", mysql_stmt_errno(handler->stmt), mysql_stmt_error(handler->stmt));*/
				return false;
			}
			return true;
		}
		/*error_log("mysql_stmt_fetch error:%s\n", mysql_stmt_error(handler->stmt));*/
		return false;
	}
	return true;
}

static bool database_free_result(database_handler_t* handler)
{
	if (mysql_stmt_free_result(handler->stmt))
	{
		if (mysql_stmt_errno(handler->stmt) == CR_SERVER_GONE_ERROR)
		{
			debug_log("try to reopen mysql\n");
			if (!database_handler_reopen(handler))
			{
				error_log("reopen failure:%d,%s\n", mysql_stmt_errno(handler->stmt), mysql_stmt_error(handler->stmt));
				return false;
			}
			debug_log("reopen success\n");
			if (mysql_stmt_free_result(handler->stmt))
			{
				error_log("mysql_stmt_free_result error:%d,%s\n", mysql_stmt_errno(handler->stmt), mysql_stmt_error(handler->stmt));
				return false;
			}
			return true;
		}
		error_log("mysql_stmt_free_result error:%d,%s\n", mysql_stmt_errno(handler->stmt), mysql_stmt_error(handler->stmt));
		return false;
	}
	return true;
}

static int database_handler_baseid = 1;
database_handler_t* database_handler_create(mysql_config_t* config)
{
	database_handler_t* handler = (database_handler_t*)malloc(sizeof(database_handler_t));
	memset(handler, 0, sizeof(database_handler_t));
	handler->id = database_handler_baseid++;
	handler->stmt = NULL;
	handler->config.host = strdup(config->host);
	handler->config.user = strdup(config->user);
	handler->config.passwd = strdup(config->passwd);
	handler->config.db = strdup(config->db);
	handler->config.port = config->port;
	if (!database_handler_reopen(handler))
	{
		database_handler_delete(handler);
		return NULL;
	}
	return handler;
}

bool database_handler_reopen(database_handler_t* handler)
{
	uint32_t timeout;
	char buf[64];
	my_bool truncation;

	if (handler->stmt)
	{
		mysql_stmt_close(handler->stmt);
		mysql_close(&handler->mysql_handler);
	}
	mysql_init(&handler->mysql_handler);
	handler->stmt = mysql_stmt_init(&handler->mysql_handler);
	
	timeout = 10;
	if (mysql_options(&handler->mysql_handler, MYSQL_OPT_CONNECT_TIMEOUT, (const char*)&timeout))
	{
		error_log("mysql_options error:%s\n", mysql_error(&handler->mysql_handler));
		return false;
	}

	timeout = 604800;
	snprintf(buf, sizeof(buf), "set wait_timeout=%d", timeout);
	if (mysql_options(&handler->mysql_handler, MYSQL_INIT_COMMAND, buf))
	{
		error_log("mysql set wait_timeout:%d error:%s\n", timeout, mysql_error(&handler->mysql_handler));
		return false;
	}

	snprintf(buf, sizeof(buf), "set interactive_timeout=%d", timeout);
	if (mysql_options(&handler->mysql_handler, MYSQL_INIT_COMMAND, buf))
	{
		error_log("mysql set interactive_timeout:%d error:%s\n", timeout, mysql_error(&handler->mysql_handler));
		return false;
	}

	snprintf(buf, sizeof(buf), "set names utf8");
	if (mysql_options(&handler->mysql_handler, MYSQL_INIT_COMMAND, buf))
	{
		error_log("mysql set names utf8 error:%s\n", mysql_error(&handler->mysql_handler));
		return false;
	}	

	truncation = true;
	if (mysql_options(&handler->mysql_handler, MYSQL_REPORT_DATA_TRUNCATION, (const char*)&truncation))
	{
		error_log("mysql MYSQL_REPORT_DATA_TRUNCATION error:%s\n", mysql_error(&handler->mysql_handler));
		return false;
	}

	if (!mysql_real_connect(&handler->mysql_handler, handler->config.host, handler->config.user, handler->config.passwd, handler->config.db, handler->config.port, NULL, 0))
	{
		error_log("connecting database:%s,%s,%s,%s,%d error: %s\n", handler->config.host, handler->config.user, handler->config.passwd, handler->config.db, handler->config.port, mysql_error(&handler->mysql_handler));
		return false;
	}

	return true;
}

void database_handler_delete(database_handler_t* handler)
{
	if (handler)
	{
		/* Todo: free mysql_config */
		if (handler->stmt)
		{
			mysql_stmt_close(handler->stmt);
			mysql_close(&handler->mysql_handler);
		}
		SAFE_FREE(handler);
	}
}

bool table_create(database_handler_t* handler, const char* tablename)
{
	char buf[960];
	snprintf(buf, sizeof(buf), "CREATE TABLE IF NOT EXISTS `%s` (`id` bigint unsigned NOT NULL, `data` longblob NOT NULL, PRIMARY KEY (`id`)) ENGINE=InnoDB DEFAULT CHARSET=utf8", tablename);
	if (!database_prepare(handler, buf)) { return false; }
	if (!database_exec(handler)) { return false; }
	return true;
}

bool table_delete(database_handler_t* handler, const char* tablename)
{
	char buf[960];
	snprintf(buf, sizeof(buf), "DROP TABLE IF EXISTS `%s`", tablename);
	if (!database_prepare(handler, buf)) { return false; }
	if (!database_exec(handler)) { return false; }
	return true;
}

bool record_create(database_handler_t* handler, const char* tablename, int64_t id, const void* data, size_t len)
{
	char buf[960];
	MYSQL_BIND param[1];
	unsigned long lengths[1] = {len};
	snprintf(buf, sizeof(buf), "INSERT INTO `%s`(id, data) VALUES(%ld, ?)", tablename, id);
	if (!database_prepare(handler, buf)) { return false; }
	memset(param, 0, sizeof(param));
	param[0].buffer_type = MYSQL_TYPE_LONG_BLOB;
	param[0].buffer = (void*)data;
	param[0].length = &lengths[0];
	if (!database_bind_param(handler, param)) { return false; }
	if (!database_exec(handler)) { return false; }
	return true;
}

bool record_delete(database_handler_t* handler, const char* tablename, int64_t id)
{
	char buf[960];
	snprintf(buf, sizeof(buf), "DELETE FROM `%s` WHERE id=%ld", tablename, id);
	if (!database_prepare(handler, buf)) { return false; }
	if (!database_exec(handler)) { return false; }
	return true;
}

bool record_update(database_handler_t* handler, const char* tablename, int64_t id, const void* data, size_t len)
{
	char buf[960];
	MYSQL_BIND param[1];
	unsigned long lengths[1] = {len};
	snprintf(buf, sizeof(buf), "UPDATE `%s` SET data=? WHERE id=%ld", tablename, id);
	if (!database_prepare(handler, buf)) { return false; }
	memset(param, 0, sizeof(param));
	param[0].buffer_type = MYSQL_TYPE_LONG_BLOB;
	param[0].buffer = (void*)data;
	param[0].length = &lengths[0];
	if (!database_bind_param(handler, param)) { return false; }
	if (!database_exec(handler)) { return false; }
	/*size = lengths[0];*/
	return true;
}

bool record_select(database_handler_t* handler, const char* tablename, int64_t id, void* data, size_t* len)
{
	char buf[960];
	MYSQL_BIND result[1];
	unsigned long lengths[1] = {0};
	snprintf(buf, sizeof(buf), "SELECT data FROM `%s` where id=%ld", tablename, id);
	if (!database_prepare(handler, buf)) { return false; }
	if (!database_exec(handler)) { return false; }
	memset(result, 0, sizeof(result));
	result[0].buffer_type = MYSQL_TYPE_LONG_BLOB;
	result[0].buffer = (void*)data;
	result[0].buffer_length = *len;
	result[0].length = &lengths[0];
	if (!database_bind_result(handler, result)) { return false; }
	if (!database_fetch(handler)) { return false; }
	if (!database_free_result(handler)) { return false; }
	*len = lengths[0];
	return true;
}

uint64_t record_insert_id(database_handler_t* handler)
{
	return mysql_stmt_insert_id(handler->stmt);
}

/* ------------------------------------------------------------------------------------------------------------------ */

/* id open_database(host, user, passwd, db, port) */
static int cc_open_database(lua_State* L)
{
	mysql_config_t config;
	database_handler_t* database_handler;
	worker_handler_t* handler = find_handler_by_stack(L);
	int args = lua_gettop(L);
	if (args < 5)
	{
		error_log("`%s` parameter lack:%d\n", __FUNCTION__, args);
		return 0;
	}
	if (!lua_isstring(L, -args))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -args)));
		return 0;
	}
	if (!lua_isstring(L, -(args-1)))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -(args-1))));
		return 0;
	}
	if (!lua_isstring(L, -(args-2)))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -(args-2))));
		return 0;
	}
	if (!lua_isstring(L, -(args-3)))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -(args-3))));
		return 0;
	}
	if (!lua_isnumber(L, -(args-4)))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -(args-4))));
		return 0;
	}

	config.host = lua_tostring(L, -args);
	config.user = lua_tostring(L, -(args-1));
	config.passwd = lua_tostring(L, -(args-2));
	config.db = lua_tostring(L, -(args-3));
	config.port = lua_tointeger(L, -(args-4));
	database_handler = database_handler_create(&config);
	if (!database_handler)
	{
		return 0;
	}
	hash_table_insert(handler->database_table, database_handler->id, database_handler);
	lua_pushinteger(L, database_handler->id);
	return 1;
}
/* void close_database(id) */
static int cc_close_database(lua_State* L)
{
	int id;
	database_handler_t* database_handler;
	worker_handler_t* handler = find_handler_by_stack(L);
	int args = lua_gettop(L);
	if (args < 1)
	{
		error_log("`%s` parameter lack:%d\n", __FUNCTION__, args);
		return 0;
	}
	if (!lua_isnumber(L, -args))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -args)));
		return 0;
	}
	id = lua_tointeger(L, -args);
	database_handler = hash_table_find(handler->database_table, id);
	if (!database_handler)
	{
		error_log("`%s` not found database:%d\n", __FUNCTION__, id);
		return 0;
	}
	hash_table_remove(handler->database_table, id);
	database_handler_delete(database_handler);
	return 0;
}
/* bool create_table(id, table_name) */
static int cc_create_table(lua_State* L)
{
	int id;
	bool retcode;
	const char* table_name;
	database_handler_t* database_handler;
	worker_handler_t* handler = find_handler_by_stack(L);
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

	if (!lua_isstring(L, -(args-1)))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -(args-1))));
		return 0;
	}

	id = lua_tointeger(L, -args);
	table_name = lua_tostring(L, -(args-1));
	database_handler = hash_table_find(handler->database_table, id);
	if (!database_handler)
	{
		error_log("`%s` not found database:%d\n", __FUNCTION__, id);
		return 0;
	}
	retcode = table_create(database_handler, table_name);
	lua_pushboolean(L, retcode ? 1 : 0);

	return 1;
}
/* bool delete_table(id, table_name) */
static int cc_delete_table(lua_State* L)
{
	int id;
	bool retcode;
	const char* table_name;
	database_handler_t* database_handler;
	worker_handler_t* handler = find_handler_by_stack(L);
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

	if (!lua_isstring(L, -(args-1)))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -(args-1))));
		return 0;
	}

	id = lua_tointeger(L, -args);
	table_name = lua_tostring(L, -(args-1));
	database_handler = hash_table_find(handler->database_table, id);
	if (!database_handler)
	{
		error_log("`%s` not found database:%d\n", __FUNCTION__, id);
		return 0;
	}
	retcode = table_delete(database_handler, table_name);
	lua_pushboolean(L, retcode ? 1 : 0);

	return 1;
}
/* bool create_record(id, table_name, sid, o) */
static int cc_create_record(lua_State* L)
{
	int id;
	int64_t sid;
	size_t len;
	const char* data;
	const char* table_name;
	bool retcode;
	database_handler_t* database_handler;
	worker_handler_t* handler = find_handler_by_stack(L);
	int args = lua_gettop(L);
	if (args < 4)
	{
		error_log("`%s` parameter lack:%d\n", __FUNCTION__, args);
		return 0;
	}

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

	if (!lua_isnumber(L, -(args-2)))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -(args-2))));
		return 0;
	}

	if (!lua_isstring(L, -(args-3)))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -(args-3))));
		return 0;
	}

	id = lua_tointeger(L, -args);
	table_name = lua_tostring(L, -(args-1));
	sid = lua_tonumber(L, -(args-2));
	len = 0;
	data = lua_tolstring(L, -(args-3), &len);
	database_handler = hash_table_find(handler->database_table, id);
	if (!database_handler)
	{
		error_log("`%s` not found database:%d\n", __FUNCTION__, id);
		return 0;
	}
	retcode = record_create(database_handler, table_name, sid, data, len);
	lua_pushboolean(L, retcode ? 1 : 0);

	return 1;
}
/* bool delete_record(id, table_name, sid) */
static int cc_delete_record(lua_State* L)
{
	int id;
	int64_t sid;
	const char* table_name;
	bool retcode;
	database_handler_t* database_handler;
	worker_handler_t* handler = find_handler_by_stack(L);
	int args = lua_gettop(L);
	if (args < 3)
	{
		error_log("`%s` parameter lack:%d\n", __FUNCTION__, args);
		return 0;
	}

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

	if (!lua_isnumber(L, -(args-2)))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -(args-2))));
		return 0;
	}

	id = lua_tointeger(L, -args);
	table_name = lua_tostring(L, -(args-1));
	sid = lua_tonumber(L, -(args-2));
	database_handler = hash_table_find(handler->database_table, id);
	if (!database_handler)
	{
		error_log("`%s` not found database:%d\n", __FUNCTION__, id);
		return 0;
	}
	retcode = record_delete(database_handler, table_name, sid);
	lua_pushboolean(L, retcode ? 1 : 0);

	return 1;
}
/* o select_record(id, table_name, sid) */
static int cc_select_record(lua_State* L)
{
	int id;
	int64_t sid;
	const char* table_name;
	bool retcode;
	char data[MAX_RECORD_SIZE];
	size_t len = sizeof(data);
	database_handler_t* database_handler;
	worker_handler_t* handler = find_handler_by_stack(L);
	int args = lua_gettop(L);
	if (args < 3)
	{
		error_log("`%s` parameter lack:%d\n", __FUNCTION__, args);
		return 0;
	}

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

	if (!lua_isnumber(L, -(args-2)))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -(args-2))));
		return 0;
	}

	id = lua_tointeger(L, -args);
	table_name = lua_tostring(L, -(args-1));
	sid = lua_tonumber(L, -(args-2));
	database_handler = hash_table_find(handler->database_table, id);
	if (!database_handler)
	{
		error_log("`%s` not found database:%d\n", __FUNCTION__, id);
		return 0;
	}
	retcode = record_select(database_handler, table_name, sid, data, &len);
	lua_pushlstring(L, data, len);

	return 1;
}
/* bool update_record(id, table_name, sid, o) */
static int cc_update_record(lua_State* L)
{
	int id;
	int64_t sid;
	size_t len;
	const char* data;
	const char* table_name;
	bool retcode;
	database_handler_t* database_handler;
	worker_handler_t* handler = find_handler_by_stack(L);
	int args = lua_gettop(L);
	if (args < 4)
	{
		error_log("`%s` parameter lack:%d\n", __FUNCTION__, args);
		return 0;
	}

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

	if (!lua_isnumber(L, -(args-2)))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -(args-2))));
		return 0;
	}

	if (!lua_isstring(L, -(args-3)))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -(args-3))));
		return 0;
	}

	id = lua_tointeger(L, -args);
	table_name = lua_tostring(L, -(args-1));
	sid = lua_tonumber(L, -(args-2));
	len = 0;
	data = lua_tolstring(L, -(args-3), &len);
	database_handler = hash_table_find(handler->database_table, id);
	if (!database_handler)
	{
		error_log("`%s` not found database:%d\n", __FUNCTION__, id);
		return 0;
	}
	retcode = record_update(database_handler, table_name, sid, data, len);
	lua_pushboolean(L, retcode ? 1 : 0);

	return 1;
}

void lua_stack_register_record_functions(lua_stack_t* stack)
{
	lua_stack_register_function(stack, "open_database", cc_open_database);	/* id open_database(host, user, passwd, db, port) */
	lua_stack_register_function(stack, "close_database", cc_close_database);/* void close_database(id) */
	lua_stack_register_function(stack, "create_table", cc_create_table);	/* bool create_table(id, table_name) */
	lua_stack_register_function(stack, "delete_table", cc_delete_table);	/* bool delete_table(id, table_name) */
	lua_stack_register_function(stack, "create_record", cc_create_record);	/* bool create_record(id, table_name, id, data) */
	lua_stack_register_function(stack, "delete_record", cc_delete_record);	/* bool delete_record(id, table_name, id) */
	lua_stack_register_function(stack, "update_record", cc_update_record);	/* bool update_record(id, table_name, id, o) */
	lua_stack_register_function(stack, "select_record", cc_select_record);	/* o select_record(id, table_name, id) */
}
