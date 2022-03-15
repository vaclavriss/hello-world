#ifndef __COMPUTATION_H__
#define __COMPUTATION_H__

#include <stdbool.h>
#include "messages.h"

void computation_init(void);
void computation_cleanup(void);

bool is_abort(void);
void abort_comp(void);
void enable_comp(void);

void get_grid_size(int *w,int *h);

void computing_stopped(void);
void computing_started(void);
bool is_computing(void);
bool is_done(void);
void not_done(void);

bool set_compute(message *msg);
bool compute(message *msg);

void update_image(int w, int h, unsigned char *img);
void update_data(const msg_compute_data *compute_data);

bool compute_pc(void);
uint8_t compute_pixel(double re, double im);
void reset_chunk_id(void);

void clear_buffer(void);

bool increase_win(void);
bool decrease_win(void);

void set_new_parameters(                
    double c_re,
    double c_im,
    double range_re_min,   
    double range_re_max,    
    double range_im_min,
    double range_im_max);

void gif(void);
void stop_gif(void);
void start_gif(void);
bool is_gif(void);

void print_grid_size(void);
#endif      