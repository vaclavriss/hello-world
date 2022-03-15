#include <unistd.h>
#include <stdio.h>

#include "xwin_sdl.h"
#include "gui.h"
#include "utils.h"
#include "computation.h"


static struct {
    int w;
    int h;
    unsigned char *img;
} gui = { .img = NULL };

void gui_init(void){
    get_grid_size(&gui.w, &gui.h);
    gui.img = my_alloc(gui.w * gui.h * 3);
    my_assert(xwin_init(gui.w, gui.h) == 0, __func__, __LINE__, __FILE__);
}

void gui_cleanup(void){
    if(gui.h) {
        free(gui.img);
        gui.img = NULL;
    }
    xwin_close();
}

void gui_refresh(void){

    if (gui.img) {
        update_image(gui.w, gui.h, gui.img);
        xwin_redraw(gui.w, gui.h, gui.img);
    }
}

void change_gui_size(void){
    free(gui.img);
    gui.img = NULL;
    get_grid_size(&gui.w, &gui.h);
    gui.img = my_alloc(gui.w * gui.h * 3);
    change_win_size(gui.w, gui.h);
}


void gui_rewrite(void){
        if (gui.img) {
        update_image(gui.w, gui.h, gui.img);
        xwin_redraw(gui.w, gui.h, gui.img);
    }
}

void get_img_size(void){
    printf("Img size: %dx%d\n", gui.w,gui.h);
}