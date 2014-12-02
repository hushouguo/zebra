/*
 * \file: crontab.c
 * \brief: Created by hushouguo at Nov 13 2014 13:18:07
 */

#include "zebra.h"

typedef struct crontab_timestamp
{
	char 			month;
	char 			day;
	char 			dayofweek;
	char 			hour;
	char 			minute;
	char 			second;
} crontab_timestamp_t;

bool crontab_timestamp_parse(crontab_timestamp_t* ts, const char* scale)
{
	char buf[128], *p_dow, *p_month, *p_day, *p_hour, *p_minute, *p_second = buf;
	snprintf(buf, sizeof(buf), "%s", scale);
	/* Todo: need to ltrip && rtrip */
	p_minute = strchr(p_second, ' ');
	if (!p_minute)
	{
		error_log("Illegal timestamp format:%s\n", scale);
		return false;
	}
	*p_minute++ = '\0';

	p_hour = strchr(p_minute, ' ');
	if (!p_hour)
	{
		error_log("Illegal timestamp format:%s\n", scale);
		return false;
	}
	*p_hour++ = '\0';

	p_day= strchr(p_hour, ' ');
	if (!p_day)
	{
		error_log("Illegal time format:%s\n", scale);
		return false;
	}
	*p_day++ = '\0';

	p_month = strchr(p_day, ' ');
	if (!p_month)
	{
		error_log("Illegal time format:%s\n", scale);
		return false;
	}
	*p_month++ = '\0';

	p_dow = strchr(p_month, ' ');
	if (!p_dow)
	{
		error_log("Illegal time format:%s\n", scale);
		return false;
	}
	*p_dow++ = '\0';

	if (*p_second == '*')
	{
		ts->second = '*';
	}
	else
	{
		ts->second = (char)strtoul(p_second, (char**)NULL, 10);
	}
	if (*p_minute == '*')
	{
		ts->minute = '*';
	}
	else
	{
		ts->minute = (char)strtoul(p_minute, (char**)NULL, 10);
	}
	if (*p_hour == '*')
	{
		ts->hour = '*';
	}
	else
	{
		ts->hour = (char)strtoul(p_hour, (char**)NULL, 10);
	}
	if (*p_day == '*')
	{
		ts->day = '*';
	}
	else
	{
		ts->day = (char)strtoul(p_day, (char**)NULL, 10);
	}
	if (*p_month == '*')
	{
		ts->month = '*';
	}
	else
	{
		ts->month = (char)strtoul(p_month, (char**)NULL, 10);
	}
	if (*p_dow == '*')
	{
		ts->dayofweek = '*';
	}
	else
	{
		ts->dayofweek = (char)atoi(p_dow);
	}

	return true;
}

void crontab_timestamp_delete(crontab_timestamp_t* ts)
{
	if (ts)
	{
		SAFE_FREE(ts);
	}
}

crontab_timestamp_t* crontab_timestamp_create(const char* scale)
{
	crontab_timestamp_t* ts = (crontab_timestamp_t*)malloc(sizeof(crontab_timestamp_t));
	memset(ts, 0, sizeof(crontab_timestamp_t));
	if (!crontab_timestamp_parse(ts, scale))
	{
		crontab_timestamp_delete(ts);
		return NULL;
	}
	return ts;
}

bool crontab_timestamp_check_expire(crontab_timestamp_t* ts, struct tm* currlocaltime)
{
	return (
			( ts->month == '*' || ts->month == (currlocaltime->tm_mon + 1) )
			&&
			( ts->day == '*' || ts->day == (currlocaltime->tm_mday) )
			&&
			( ts->dayofweek == '*' || ts->dayofweek == (currlocaltime->tm_wday) )
			&&
			( ts->hour == '*' || ts->hour == (currlocaltime->tm_hour) )
			&&
			( ts->minute == '*' || ts->minute == (currlocaltime->tm_min) )
			&&
			( ts->second == '*' || ts->second == (currlocaltime->tm_sec) )
	   );
}

/*********************************************************************************************************************/

typedef struct crontab_task
{
	char*					id;
	crontab_timestamp_t*	ts;
	crontab_callback		cb;
	void* 					object;
	void* 					arg;
} crontab_task_t;

void crontab_task_delete(crontab_task_t* task)
{
	if (task)
	{
		crontab_timestamp_delete(task->ts);
		SAFE_FREE(task->id);
		SAFE_FREE(task);
	}
}

crontab_task_t* crontab_task_create(const char* id, const char* scale, crontab_callback cb, void* object, void* arg)
{
	crontab_task_t* task = (crontab_task_t*)malloc(sizeof(crontab_task_t));
	task->id = strdup(id);
	task->ts = crontab_timestamp_create(scale);
	if (!task->ts)
	{
		crontab_task_delete(task);
		return NULL;
	}
	task->cb = cb;
	task->object = object;
	task->arg = arg;
	return task;
}

/*********************************************************************************************************************/

crontab_t* crontab_create()
{
	crontab_t* crontab = (crontab_t*)malloc(sizeof(crontab_t));
	memset(crontab, 0, sizeof(crontab_t));
	crontab->one_second_timer = time(0);
	crontab->crontask_table = hash_table_create(16);
	return crontab;
}

void crontab_delete(crontab_t* crontab)
{
	if (crontab)
	{
		hash_table_delete(crontab->crontask_table);
		SAFE_FREE(crontab);
	}
}

bool crontab_register(crontab_t* crontab, const char* id, const char* scale, crontab_callback cb, void* object, void* arg)
{
	crontab_task_t* task;
	if (!id)
	{
		error_log("register crontask id is nil\n");
		return false;
	}
	if (!scale)
	{
		error_log("register crontask scale is nil\n");
		return false;
	}
	if (!cb)
	{
		error_log("register crontask start_routine is nil\n");
		return false;
	}
	task = crontab_task_create(id, scale, cb, object, arg);
	if (!task)
	{
		return false;
	}
	return hash_table_insert(crontab->crontask_table, hash_string(id, strlen(id)), task);
}

bool crontab_unregister(crontab_t* crontab, const char* id)
{
	return hash_table_remove(crontab->crontask_table, hash_string(id, strlen(id)));
}

void crontab_loop(crontab_t* crontab)
{
	time_t now = time(0);
	if ((now - crontab->one_second_timer) >= 1)
	{
		size_t n;
		struct tm currlocaltime;
		localtime_r(&now, &currlocaltime);
		for (n = 0; n < crontab->crontask_table->nodesize; ++n)
		{
			hash_node_t* node = crontab->crontask_table->nodes[n];
			while (node)
			{
				crontab_task_t* task = (crontab_task_t*)node->value;
				if (crontab_timestamp_check_expire(task->ts, &currlocaltime))
				{
					task->cb(task->object, task->arg);
				}
				node = node->next;
			}
		}
		crontab->one_second_timer = now;
	}
}

