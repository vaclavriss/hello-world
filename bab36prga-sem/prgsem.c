#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

#include "main.h"
#include "event_queue.h"
#include "utils.h"
#include "prg_io_nonblock.h"
#include "messages.h"
#include "gui.h"
#include "computation.h"

#define EXIT_SUCCESS 0
#define IO_READ_TIMEOUT_MS 100
#define IN_PIPE "/tmp/computational_module.out"
#define OUT_PIPE "/tmp/computational_module.in"

void* keyboard_thread(void *d);
void* read_pipe_thread(void *d);

int main(int argc, char *argv[])
{   
    int ret = EXIT_SUCCESS;
    const char *pipe_in = IN_PIPE;      
    const char *pipe_out = OUT_PIPE;

    int pipe_in_int = io_open_read(pipe_in);
    int pipe_out_int = io_open_write(pipe_out);

    my_assert(pipe_in_int != -1 && pipe_out_int != -1, __func__, __LINE__, __FILE__);

    enum { KEYBOARD_THRD, READ_PIPE_THDR, MAIN_THDR, NUM_THREADS };
    void *  (*thrd_functions[])(void *) = { keyboard_thread, read_pipe_thread, main_thread };
    const char *thrd_names[] = { "Keyboard thread", "Pipe out thread", "Main thread" };
    void* thrd_data[NUM_THREADS] = {};
    thrd_data[READ_PIPE_THDR] = &pipe_in_int;
    thrd_data[MAIN_THDR] = &pipe_out_int;
    
    //thread_data[]
    
    pthread_t threads [NUM_THREADS];

    for(int i = 0; i < NUM_THREADS; ++i)
    {
       int r = pthread_create(&threads[i], NULL, thrd_functions[i], thrd_data[i]);
       printf("Create thread '%s'  %s\n", thrd_names[i], (r == 0 ? "OK" : "FAIL"));
    }

    int *ex;

    for(int i = 0; i < NUM_THREADS; ++i)
    {
       printf("Call join to thread '%s'\n", thrd_names[i]);
       int r = pthread_join(threads[i], (void*)&ex);
       printf("Joining the thread '%s' has been %s\n", thrd_names[i], (r == 0 ? "OK" : "FAIL"));
    }
    
    io_close(pipe_in_int);
    io_close(pipe_out_int);

    return ret;
}

void* keyboard_thread(void *d){ 
    call_termios(0);
    int c;  
    event event;
    while((c = getchar()) != 'q' )
    {
        event.type = EV_TYPE_NUM; 
        switch(c)
        {
            case 'g':
                event.type = EV_GET_VERSION;
                break;
            case 'a':
                event.type = EV_ABORT;
                break;
            case 's':
                event.type = EV_SET_COMPUTE;
                call_termios(1);

                char c;
                double num = 0;
                
                double c_re; //constant_real
                double c_im; //constant_imaginary
    

                double range_re_min;        //interval realne roviny
                double range_re_max;    
                double range_im_min;        //interval komplexni roviny
                double range_im_max;
                
                

                fprintf(stderr,"Do you want to change parameter c or zoom? (y/n)\n");
                scanf("%c", &c);
                if (c == 'y'){
                    fprintf(stderr,"To keep variable same press '123'\n");
                    
                    fprintf(stderr,"Paramater c re. part: ");
                    scanf("%lf",&num);
                    if (num == 123){
                        fprintf(stderr,"Not changed\n");
                    }
                    c_re = num;

                    fprintf(stderr,"Paramater c im. part: ");
                    scanf("%lf",&num);
                    if (num == 123){
                        fprintf(stderr,"Not changed\n");
                    }
                    c_im = num;

                    fprintf(stderr,"Min. Range, re. part: ");
                    scanf("%lf",&num);
                    if (num == 123){
                        fprintf(stderr,"Not changed\n");
                    }
                    range_re_min = num;
                    
                    fprintf(stderr,"Max. Range, re. part: ");
                    scanf("%lf",&num);
                    if (num == 123){
                        fprintf(stderr,"Not changed\n");
                    }
                    range_re_max = num;

                    fprintf(stderr,"Min. Range, im. part: ");
                    scanf("%lf",&num);
                    if (num == 123){
                        fprintf(stderr,"Not changed\n");
                    }
                    range_im_min = num;

                    fprintf(stderr,"Max. Range, im. part: ");
                    scanf("%lf",&num);
                    if (num == 123){
                        fprintf(stderr,"Not changed\n");
                    }
                    range_im_max = num;
                    
                    set_new_parameters(                
                    c_re,
                    c_im,
                    range_re_min,   
                    range_re_max,    
                    range_im_min,
                    range_im_max);

                }
                else if (c == 'n'){
                    fprintf(stderr,"Nothing changed.\n");
                }
                else{ warn("Unknwown character. Ending setting"); }

                call_termios(0);


                break;
            case '1':
                event.type = EV_COMPUTE;
                break;
            case 'c':
                event.type = EV_COMPUTE_CPU;
                break;
            case 'r':
                event.type = EV_RESET_CHUNK;
                break;
            case 'l':
                event.type = EV_CLEAR_BUFFER; 
                break;
            case 'p':
                event.type = EV_REFRESH;
            case '+':  
                event.type = EV_INCREASE_RES;
                break;
            case '-':
                event.type = EV_DECREASE_RES;
                break;
            case 'u':
                event.type = EV_GIF_START;
                break;
            case 'i':
                event.type = EV_GIF_STOP;
                break;
            case 'h':
                fprintf(stderr,"***************************************\n");
                fprintf(stderr,"s - set compute\n");
                fprintf(stderr,"c - compute on pc\n");
                fprintf(stderr,"1 - compute on module\n");
                fprintf(stderr,"r - reset chunk id\n");
                fprintf(stderr,"l - clear buffer (and screen)\n");
                fprintf(stderr,"p - print current buffer\n");
                fprintf(stderr,"+ - increase win size\n");
                fprintf(stderr,"- - decrease win size\n");
                fprintf(stderr,"u - start gif\n");
                fprintf(stderr,"i - stop gif\n");
                fprintf(stderr,"***************************************\n");

            default:
                //warn("pressed wrong button");
                break; 

        }
        if (event.type !=  EV_TYPE_NUM) {
            queue_push(event);
        }
    } //end while
    set_quit();
    event.type = EV_QUIT;
    queue_push(event);
    call_termios(1);

    return NULL;
}

void* read_pipe_thread(void *d){
    my_assert( d != NULL, __func__, __LINE__, __FILE__); 
    int pipe_in = *(int*)d;
    bool end = false;
    uint8_t msg_buf[sizeof(message)];
    int i = 0;
    int len = 0;
    event event;

    unsigned char c;
    while(io_getc_timeout(pipe_in, IO_READ_TIMEOUT_MS, &c) > 0);

    while (!end) {
        int r = io_getc_timeout(pipe_in, IO_READ_TIMEOUT_MS, &c);
        
        if (r > 0){
            if(i == 0){
                if(get_message_size(c, &len)) {
                    msg_buf[i++] = c;
                } else {    
                    fprintf(stderr," ERROR: unknown message type\n");
                }
            }
            else{
                msg_buf[i++] = c; 
            }
            if (len > 0 && i == len){
                message *msg =  my_alloc(sizeof(message)); 
                if (parse_message_buf(msg_buf,len,msg)){
                    event.type = EV_PIPE_IN_MESSAGE;
                    event.data.msg = msg;
                    queue_push(event); 
                }
                else{
                    printf("ERROR: Cannot parse message type!\n");
                    free(msg);
                }
                i = 0;
                len = 0;
            }
        }
        else if (r == 0) { } 
        else {
            fprintf(stderr," ERROR: reading from pipe FAILED\n");
            set_quit();
            event.type = EV_QUIT; 
            queue_push(event); 
        }
        end = is_quit();
    } //end while

    return NULL;
}

