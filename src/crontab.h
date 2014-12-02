/*
 * \file: crontab.h
 * \brief: Created by hushouguo at Nov 13 2014 13:04:21
 */
 
#ifndef __CRONTAB_H__
#define __CRONTAB_H__

typedef bool (*crontab_callback)(void*, void*);
typedef struct crontab
{
	time_t					one_second_timer;
	hash_table_t*			crontask_table;
} crontab_t;

crontab_t* crontab_create();
void crontab_delete(crontab_t* crontab);
/*
 * `scale` format: [second] [minute] [hour] [day] [month] [dayofweek]
 *	 rule 1: `*`: means any scale
 *	 rule 2: `unless [dayofweek] not defined, otherwise [day] will not take effect`
 *	 example: `30 * 0 * * *` every month, every day, on 0 hour, every minute, 30 second trigger crontab
 */
bool crontab_register(crontab_t* crontab, const char* id, const char* scale, crontab_callback cb, void* object, void* arg);
bool crontab_unregister(crontab_t* crontab, const char* id);
void crontab_loop(crontab_t* crontab);

#endif
