/*
 * \file: networking.h
 * \brief: Created by hushouguo at Thu 16 Oct 2014 10:54:00 AM CST
 */
 
#ifndef __NETWORKING_H__
#define __NETWORKING_H__

bool setblocking(int fd, bool block);
bool interrupted();
bool wouldblock();
bool connectionlost();

void lua_stack_register_network_functions(lua_stack_t* stack);

#endif
