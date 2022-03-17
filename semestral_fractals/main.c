#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>

#include "main.h"
#include "event_queue.h"
#include "utils.h"
#include "messages.h"
#include "prg_io_nonblock.h"
#include "computation.h"
#include "gui.h"
#include "xwin_sdl.h"




static void process_pipe_message(event * const event);

void* main_thread(void *d){
    my_assert(d != NULL, __func__, __LINE__, __FILE__);
    int pipe_out = *(int*)d;
    message msg;
    uint8_t msg_buf[sizeof(message)];
    int msg_len = 1;
    bool quit = false;

    computation_init();
    gui_init();
    do{
        event ev = queue_pop();
        msg.type = MSG_NBR;
        switch(ev.type){
            case EV_QUIT:
                break;
            case EV_GET_VERSION:
                msg.type = MSG_GET_VERSION;
                break;
            case EV_SET_COMPUTE:
                info(set_compute(&msg) ? "set compute" : "set compute FAILED");
                break;
            case EV_COMPUTE:
                enable_comp();
                info(compute(&msg) ? "compute" : "compute FAILED");
                break;
            case EV_COMPUTE_CPU:
                info(compute_pc() ? "compute pc" : "compute pc FAILED");
                gui_refresh();
                info("computation done");
                break;
            case EV_ABORT:
                msg.type = MSG_ABORT;
                abort_comp();
                //computing_stopped();
                break;
            case EV_RESET_CHUNK:
                fprintf(stderr,is_abort() ? "INFO: Chunk reset request\n" : "WARN: Chunk reset request discarded, it is currently computing\n");
                if (is_abort()) { 
                    reset_chunk_id();
                    //gui_refresh();
                 }
                break;
            case EV_PIPE_IN_MESSAGE:
                process_pipe_message(&ev);
                break;
            case EV_CLEAR_BUFFER:
                clear_buffer();
                gui_refresh();
                info("buffer cleared");
                break;
            case EV_REFRESH:
                update_data(&msg.data.compute_data);
                gui_refresh();
                info("Buffer printed");
                break;
            case EV_INCREASE_RES:
                if (is_computing() || is_abort()){
                    debug("Cannot change size - device is computing");
                }
                info(increase_win() ? "window increased" : "cannot increase - window reached maximum size");
                break;
            case EV_DECREASE_RES:
                if (is_computing() || is_abort()){
                    debug("Cannot changge size - device is computing");
                }
                info(decrease_win() ? "window decreased" : "cannot decrease - window reached minimum size");
                break;
            case EV_GIF:
                if(is_gif()){
                    gif();
                    event gif = { .type = EV_GIF };
                    queue_push(gif);
                }
                break;
            case EV_GIF_START:
                start_gif();
                event start = { .type = EV_GIF };
                queue_push(start);
                break;
            case EV_GIF_STOP:
                stop_gif();
                break;
            default:
                break;
        } //switch end
        if (msg.type != MSG_NBR) {
            my_assert(fill_message_buf(&msg, msg_buf,sizeof(message), &msg_len), __func__, __LINE__, __FILE__ );
            if (write(pipe_out, msg_buf, msg_len) == msg_len){
                debug("send date to pipe_out");
            }
            else{
                //fprintf(stderr, "ERROR: Send fail, len - %d !\n",l);
                error("send fail");
            }
        }
        quit = is_quit();
    }while(quit != true);
    gui_cleanup();    
    computation_cleanup();
    return NULL;           

}


static void process_pipe_message(event * const ev){
    my_assert(ev != NULL && ev->type == EV_PIPE_IN_MESSAGE && ev->data.msg, __func__, __LINE__, __FILE__);
    ev->type =  EV_TYPE_NUM;
    const message * msg = ev->data.msg; 
    switch(msg->type) {
       case MSG_OK:
            info("Receive OK from Module");
            break;
        case MSG_VERSION:
            fprintf(stderr,"INFO: Module version %d.%d-p%d\n", msg->data.version.major, msg->data.version.minor, msg->data.version.patch);
            break;
        case MSG_COMPUTE_DATA:
            update_data(&msg->data.compute_data);
            break;  
        case MSG_DONE:
            gui_refresh();
            if(is_done()){
                info("Computation done");
                not_done();
            }
            else{ 
                event event = { .type = EV_COMPUTE };
                queue_push(event);
            } 
            break;
        case MSG_ABORT:
            info("computation aborted");
            abort_comp();
            computing_stopped();
            break;
        case MSG_ERROR:
            warn("WARN: Receive error from Module\n");
        default:
            fprintf(stderr,"ERROR: Unhandled message type %d\n", msg->type);
            break;  
    }
    free(ev->data.msg);
    ev->data.msg = NULL;
}
