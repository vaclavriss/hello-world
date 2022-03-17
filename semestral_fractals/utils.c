#include <stdio.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>  // for STDIN_FILENO
#include "utils.h"

#define CANNOT_MALLOC 101
#define ASSERT_FAIL 105



void my_assert(bool r, const char *func_name, int line, const char *file_name){
    if(r == false){
        fprintf(stderr, "ERROR: My assert FAIL: %s() line %d in %s\n", func_name, line, file_name);   
        exit(ASSERT_FAIL); 
    }
}

void *my_alloc(size_t size){
    void *r = malloc(size);
    if (r == NULL){
        fprintf(stderr, "ERROR: cannot malloc!\n");
        exit(CANNOT_MALLOC);
    }
    return r;
}

void call_termios(int reset)
{
   static struct termios tio, tioOld;
   tcgetattr(STDIN_FILENO, &tio);
   if (reset) {
      tcsetattr(STDIN_FILENO, TCSANOW, &tioOld);
   } else {
      tioOld = tio; //backup
      cfmakeraw(&tio);
      tio.c_oflag |= OPOST;
      tcsetattr(STDIN_FILENO, TCSANOW, &tio);
   }
}

void error(char *str){
    fprintf(stderr,"ERROR: %s \n",str);
}
void debug(char *str){
    fprintf(stderr,"DEBUG: %s \n",str);
}
void warn(char *str){
    fprintf(stderr,"WARN: %s \n",str);
}
void info(char *str){
    fprintf(stderr,"INFO: %s \n",str);
}


