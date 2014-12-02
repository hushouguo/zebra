/*
 * \file: function.h
 * \brief: Created by hushouguo at Nov 07 2014 10:07:29
 */
 
#ifndef __FUNCTION_H__
#define __FUNCTION_H__

const char* domain_lookup(const char* domain);

void* encrylight(void* p, size_t size);
void* decrylight(void* p, size_t size);

bool check_system_limits();

#endif
