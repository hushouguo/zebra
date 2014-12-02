/*
 * \file: configure.h
 * \brief: Created by hushouguo at Nov 09 2014 03:30:44
 */
 
#ifndef __CONFIGURE_H__
#define __CONFIGURE_H__

bool conf_init(const char* file);
void conf_clean();
void conf_print();
bool conf_set_value(const char* key, const char* value);
const char* conf_get_value(const char* key);

#endif
