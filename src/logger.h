/*
 * \file: logger.h
 * \brief: Created by hushouguo at Nov 12 2014 13:18:20
 */
 
#ifndef __LOGGER_H__
#define __LOGGER_H__

typedef enum
{
	NONE = 0, 
	BLACK = 30,	
	RED = 31, 		
	GREEN = 32, 	
	BROWN = 33, 
	BLUE = 34, 		
	MAGENTA = 35, 	
	CYAN = 36, 
	GREY = 37,
	LRED = 41, 	
	LGREEN = 42,	
	YELLOW = 43,	
	LBLUE = 44,	
	LMAGENTA = 45,	
	LCYAN = 46,		
	WHITE = 47
}COLOR;

#define error_log(fmt, ...)\
	do {\
		fprintf(stderr, "\x1b[%d;1m[%s][ERR][%s:%d] "fmt"\x1b[0m", LRED-10, timestamp(time(0), "%02H:%02M:%02S"), __FILE__, __LINE__, ##__VA_ARGS__);\
	}while(0)

#define alarm_log(fmt, ...)\
	do {\
		fprintf(stderr, "\x1b[%d;1m[%s][ALM][%s:%d] "fmt"\x1b[0m", YELLOW-10, timestamp(time(0), "%02H:%02M:%02S"), __FILE__, __LINE__, ##__VA_ARGS__);\
	}while(0)
		
#define debug_log(fmt, ...)\
	do {\
		fprintf(stdout, "[%s] "fmt, timestamp(time(0), "%02H:%02M:%02S"), ##__VA_ARGS__);\
	}while(0)

#define lua_error_log(fmt, ...)\
	do {\
		fprintf(stderr, "\x1b[%d;1m[%s][LUA ERR][%s:%d] "fmt"\x1b[0m", LRED-10, timestamp(time(0), "%02H:%02M:%02S"), __FILE__, __LINE__, ##__VA_ARGS__);\
	}while(0)

#define lua_alarm_log(fmt, ...)\
	do {\
		fprintf(stderr, "\x1b[%d;1m[%s][LUA ALM][%s:%d] "fmt"\x1b[0m", YELLOW-10, timestamp(time(0), "%02H:%02M:%02S"), __FILE__, __LINE__, ##__VA_ARGS__);\
	}while(0)
		
#define lua_debug_log(fmt, ...)\
	do {\
		fprintf(stdout, "[%s][LUA] "fmt, timestamp(time(0), "%02H:%02M:%02S"), ##__VA_ARGS__);\
	}while(0)

int logger_open(const char* filename);
void logger_close(int fd);
void logger_flush(int fd, COLOR color, const char* text, size_t len);

void lua_stack_register_logger_functions(lua_stack_t* stack);

#endif
