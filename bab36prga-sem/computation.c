#include "computation.h"
#include "xwin_sdl.h"
#include "messages.h"
#include "utils.h"
#include "gui.h"
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>

#define NUM_RES 7

int res[NUM_RES][4] ={
    {160,120,16,12},
    {320,240,32,24},
    {440,330,44,33},
    {540,405,54,40},
    {640,480,64,48},
    {800,600,80,60},
    {1240,930,124,93}
};

struct {
    double c_re; //constant_real
    double c_im; //constant_imaginary
    int n;
    int r;

    double range_re_min;        //interval realne roviny
    double range_re_max;    
    double range_im_min;        //interval komplexni roviny
    double range_im_max;

    int grid_w;                 //rozliseni obrazku 
    int grid_h;
    int grid_idx;
    
    int cur_x;                  //souradnice kurzoru 
    int cur_y;                  

    double d_re;      
    double d_im;

    int nbr_chunks;             //number of chunks (kousku)
    int cid;                    //chunk id
    double chunk_re;            //zacatek souradnic daneho chunku - realna slozka
    double chunk_im;            //zacatek souradnic daneho chunku - imaginarni slozka

    uint8_t chunk_n_re;         //number of chunks v realne ose 
    uint8_t chunk_n_im;         //number of chunks v imaginarni ose

    uint8_t *grid;              //mrizka
    bool computing;             //pocitam?
    bool abort;
    bool done;                  //zpocital?
    
    
    
    bool stop_gif; 
    bool is_gif;
    
    double gif_max_re;
    double gif_max_im;
    double gif_min_re;
    double gif_min_im;  
 
} comp = {
    .c_re = -0.4,
    .c_im = 0.6, 

    .n = 60,
    .r = 2,
    .range_re_min = -1.6,
    .range_re_max = 1.6,
    .range_im_min = -1.1,
    .range_im_max = 1.1,

    .grid = NULL,
    .grid_w = 640,
    .grid_h = 480,
    .grid_idx = 4,

    .chunk_n_re = 64,
    .chunk_n_im = 48,

    .computing = false,
    .abort = false,
    .done = false,
    
    
    .gif_max_re = 0.6,
    .gif_max_im = 1.6,
    .gif_min_re = -1.4,
    .gif_min_im = -0.4,

    .stop_gif = true,
    .is_gif = false
};

void computation_init(void){
    comp.grid = my_alloc(comp.grid_w * comp.grid_h);
    clear_buffer();
    comp.d_re = (comp.range_re_max - comp.range_re_min)/(1. *comp.grid_w); 
    comp.d_im = -(comp.range_im_max - comp.range_im_min)/(1. *comp.grid_h);
    comp.nbr_chunks = (comp.grid_w * comp.grid_h) / (comp.chunk_n_re * comp.chunk_n_im);
}
void computation_cleanup(void){
    if(comp.grid){
        free(comp.grid);
    }
    comp.grid =  NULL;
}

bool is_computing(void){ return comp.computing; }
bool is_done(void){return comp.done; }

bool set_compute(message *msg) {
    
    my_assert(msg != NULL, __func__, __LINE__,__FILE__); 
    bool r = !is_computing();
    
    if(r){;
        msg->type = MSG_SET_COMPUTE;
        msg->data.set_compute.c_re = comp.c_re;
        msg->data.set_compute.c_im = comp.c_im;
        msg->data.set_compute.d_re = comp.d_re;
        msg->data.set_compute.d_im = comp.d_im;
        msg->data.set_compute.n = comp.n;
        comp.done = false;
    }
    return r;
 }

bool compute(message *msg){


    my_assert(msg != NULL, __func__, __LINE__,__FILE__);
    if(!is_computing()){  
        comp.cid = 0;
        comp.computing = true;
        comp.cur_x = 0;
        comp.cur_y = 0;
        comp.chunk_re = comp.range_re_min;
        comp.chunk_im = comp.range_im_max;
        msg->type = MSG_COMPUTE;
    } 
    else{
        comp.cid += 1;
        if(comp.cid < comp.nbr_chunks){
            comp.cur_x += comp.chunk_n_re;
            comp.chunk_re += comp.chunk_n_re *comp.d_re;
            if(comp.cur_x >= comp.grid_w){
                comp.chunk_re = comp.range_re_min;
                comp.chunk_im += comp.chunk_n_im * comp.d_im;
                comp.cur_x = 0;
                comp.cur_y += comp.chunk_n_im;
            } 
            msg->type = MSG_COMPUTE;
        }
        //next chunk
    }

    if(comp.computing && msg->type == MSG_COMPUTE) {
        msg->data.compute.cid = comp.cid;
        msg->data.compute.re = comp.chunk_re;
        msg->data.compute.im = comp.chunk_im;
        msg->data.compute.n_re = comp.chunk_n_re;
        msg->data.compute.n_im = comp.chunk_n_im;
    }

    return is_computing();
}


void update_image(int w, int h, unsigned char *img){
    my_assert(img && comp.grid && w == comp.grid_w && h == comp.grid_h, __func__, __LINE__,__FILE__);
    for (int i = 0; i <w*h;i++){
        const double t = 1. * comp.grid[i] / (comp.n + 1.0);
        *(img++) = 9*(1-t)*t*t*t * 255;
        *(img++) = 15 * (1-t)*(1-t)* t * t * 255;
        *(img++) = 8.5 * (1-t)*(1-t)*(1-t)*t*255;
    }
}

void update_data(const msg_compute_data *compute_data){
    if (compute_data->cid == comp.cid) {
        const int idx = comp.cur_x + compute_data->i_re + (comp.cur_y + compute_data->i_im) * comp.grid_w;
        if(idx >= 0 && idx < (comp.grid_w *  comp.grid_h)){
            comp.grid[idx] = compute_data->iter;
        }
        if((comp.cid +1) >= comp.nbr_chunks && (compute_data->i_re + 1) == comp.chunk_n_re && (compute_data->i_im + 1) == comp.chunk_n_im){
            comp.done = true;
            comp.computing = false;
        }     
    }
    else{
        warn("Received chunk with unexpected chunk id");
    }

}

void get_grid_size(int *w,int *h){
    *w = comp.grid_w;
    *h = comp.grid_h;
}

void reset_chunk_id(){
    computation_cleanup();
    computation_init();
    comp.cid = 0;
    comp.cur_x = 0;
    comp.cur_y = 0;
    comp.chunk_re = comp.range_re_min;
    comp.chunk_im = comp.range_im_max;
}

void abort_comp(void){ comp.abort = true; }
void enable_comp(void){ comp.abort = false; }
bool is_abort(void){ return comp.abort; }

void computing_stopped(void){ comp.computing = false; }
void computing_started(void){ comp.computing = true; }

uint8_t compute_pixel(double re, double im){  

    uint8_t iter = 0;

    for (iter = 1; iter <= comp.n; iter++){
        double old_re = re;
        re = re*re-im*im;
        im = 2*old_re*im;

        re += comp.c_re;
        im += comp.c_im;

        if (sqrt(re*re + im*im) >= 2){
            return iter;
        } 
    }

    return iter;
}

bool compute_pc(void){
    
    bool r = true;
    int p = 0;
    if(comp.grid_idx == 4){
        for(double y = comp.range_im_max; y > (comp.range_im_min); y+= comp.d_im){
            for(double x = comp.range_re_min; x < (comp.range_re_max-comp.d_re); x += comp.d_re){
                if(p == (comp.grid_w*comp.grid_h)){}
                else{
                    comp.grid[p] = compute_pixel(x,y);
                    p +=1;
                }
            }
        }
    }
    else{
        for(double y = comp.range_im_max; y > (comp.range_im_min); y+= comp.d_im){
            for(double x = comp.range_re_min; x < (comp.range_re_max); x += comp.d_re){
                if(p == (comp.grid_w*comp.grid_h)){}
                else{
                    comp.grid[p] = compute_pixel(x,y);
                    p +=1;
                }
            }
        }
    }

    comp.computing = false;
    return r;
}


void clear_buffer(void){
    for(int i = 0; i < comp.grid_w*comp.grid_h; i++){
            comp.grid[i] = 0;
    }
}

void not_done(void) { comp.done = false; }


bool increase_win(void){
    
    bool r = true;

    if ((comp.grid_idx+1) == NUM_RES){
        r = false;
        return r;    
    }
    else {
    comp.grid_idx += 1;
    comp.grid_w = res[comp.grid_idx][0];
    comp.grid_h = res[comp.grid_idx][1];
    comp.chunk_n_re = res[comp.grid_idx][2];
    comp.chunk_n_im = res[comp.grid_idx][3];
    }
    change_gui_size();
    computation_cleanup();
    computation_init();
    clear_buffer();
    gui_refresh();
    return true;   
}

bool decrease_win(void){
    
    bool r = true;

    if ((comp.grid_idx) == 0){
        r = false;
        return r;    
    }
    else {
    comp.grid_idx -= 1;
    comp.grid_w = res[comp.grid_idx][0];
    comp.grid_h = res[comp.grid_idx][1];
    comp.chunk_n_re = res[comp.grid_idx][2];
    comp.chunk_n_im = res[comp.grid_idx][3];
    }
    change_gui_size();
    computation_cleanup();
    computation_init();
    clear_buffer();
    gui_refresh();
    return true;   
}

void print_grid_size(void){
    printf("Grid size: %dx%d\n",comp.grid_w,comp.grid_h);
}


void set_new_parameters(               
    double c_re,
    double c_im,
    double range_re_min,   
    double range_re_max,    
    double range_im_min,
    double range_im_max){


    if(c_re != 123){ comp.c_re = c_re; }
    if(c_im!= 123){ comp.c_im = c_im; }
    if(range_re_min != 123){ comp.range_re_min = range_re_min; }
    if(range_re_max != 123){ comp.range_re_max = range_re_max; }
    if(range_im_min != 123){ comp.range_im_min = range_im_min; }
    if(range_im_max != 123){ comp.range_im_max = range_im_max; }

    computation_init();
}

void stop_gif(void){ 
    
    comp.c_re = -0.4;
    comp.c_im = 0.6;
    comp.is_gif = false;
 }


void start_gif(void){ 
    
    comp.is_gif = true;
    comp.c_re -= 1;
    comp.c_im -= 1;
 }


bool is_gif(void){ return comp.is_gif; }

void gif(void){ 

   if (compute_pc() == true && comp.is_gif == true){
        gui_refresh();
        sleep(0.04);
        comp.c_re += 0.01;
        if(comp.c_re > comp.gif_max_re){
            comp.c_re = comp.gif_min_re;
        }
        comp.c_im += 0.01;
        if(comp.c_im > comp.gif_max_im){
            comp.c_im =comp.gif_min_im;
        } 
    }
    
}