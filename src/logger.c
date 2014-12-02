/*
 * \file: logger.c
 * \brief: Created by hushouguo at Nov 12 2014 13:45:43
 */

#include "zebra.h"

static void logger_set_color(int fd, COLOR color)
{
	char buffer[32];
	snprintf(buffer, sizeof(buffer), "\x1b[%d%sm", color >= LRED ? (color - 10) : color, color >= LRED ? ";1" : "");
	write(fd, buffer, strlen(buffer));
}

static void logger_reset_color(int fd)
{
	const char* s = "\x1b[0m";
	write(fd, s, strlen(s));
}

static bool logger_create_folder(const char* path)
{
	char name[1024], *dir = name;
	strncpy(name, path, sizeof(name));
	while (1)
	{
		char* tailer = strchr(dir, '/');
		if (!tailer) { break; }
		*tailer++ = '\0';

		if (access(dir, X_OK) != 0)
		{
			if (access(dir, F_OK) == 0)
			{
				error_log("directory:%s exist, but can't be executed\n", dir);
				return false;
			}
			if (mkdir(dir, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH) == -1)
			{
				error_log("cannot mkdir:%s,%d,%s\n", dir, errno, errstring(errno));
				return false;
			}
		}

		dir = tailer;
	}
	return true;
}

int logger_open(const char* filename)
{
	int fd;
	if (!logger_create_folder(filename)) { return -1; }
	fd = open(filename, O_CREAT|O_APPEND|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP);
	if (fd < 0)
	{
		error_log("cannot open file:%s,%d,%s\n", filename, errno, errstring(errno));
	}
	else
	{
		/*crontab_register(shard->crontab, "logger", "30 * * * * *", cb, (void*)(long)fd, (void*)filename);*/
	}
	return fd;
}

void logger_close(int fd)
{
	close(fd);
}

void logger_flush(int fd, COLOR color, const char* text, size_t len)
{
	if (color != NONE)
	{
		logger_set_color(fd, color);
		write(fd, text, len);
		logger_reset_color(fd);
	}
	else
	{
		write(fd, text, len);
	}	
}

/* fd open_logger(filename) */
static int cc_open_logger(lua_State* L)
{
	const char* filename;
	int fd, args = lua_gettop(L);
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

	filename = lua_tostring(L, -args);
	fd = logger_open(filename);
	lua_pushinteger(L, fd);
	return 1;
}

/* void close_logger(fd) */
static int cc_close_logger(lua_State* L)
{
	int fd, args = lua_gettop(L);
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

	fd = lua_tointeger(L, -args);
	logger_close(fd);
	return 0;
}

/* void flush_logger(fd, color, text) */
static int cc_flush_logger(lua_State* L)
{
	const char* text;
	size_t len;
	int fd, color, args = lua_gettop(L);
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

	if (!lua_isnumber(L, -(args-1)))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -(args-1))));
		return 0;
	}

	if (!lua_isstring(L, -(args-2)))
	{
		error_log("`%s` parameter error:%s\n", __FUNCTION__, lua_typename(L, lua_type(L, -(args-2))));
		return 0;
	}

	fd = lua_tointeger(L, -args);
	color = lua_tointeger(L, -(args-1));
	text = lua_tolstring(L, -(args-2), &len);
	logger_flush(fd, color, text, len);
	return 0;
}

void lua_stack_register_logger_functions(lua_stack_t* stack)
{
	/* register color macro */
	lua_stack_register_function(stack, "open_logger", cc_open_logger);		/* fd open_logger(filename) */
	lua_stack_register_function(stack, "close_logger", cc_close_logger);	/* void close_logger(fd) */
	lua_stack_register_function(stack, "flush_logger", cc_flush_logger);	/* void flush_logger(fd, color, text) */
}

