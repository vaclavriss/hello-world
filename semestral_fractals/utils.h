#ifndef __UTILS_H__
#define __UTILS_H__


#include <stdbool.h>
#include <stdlib.h>


void my_assert(bool r, const char *func_name, int line, const char *file_name);
void *my_alloc(size_t size);
void call_termios(int reset);

void debug(char *  str);
void error(char * str);
void warn(char *str);
void info(char * str);

#endif