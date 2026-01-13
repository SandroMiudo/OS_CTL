#include "display_driver.h"
#include "display_api.h"
#include "frame_buffer.h"

void display_draw_image(const cmd_draw_image_t* cmd){
    if (global_driver && global_driver->draw_image)
        global_driver->draw_image(
            cmd->x, cmd->y,
            cmd->width, cmd->height,
            cmd->buffer
        );
}

void display_set_pixel(const cmd_set_pixel_t* cmd){
    if (global_driver && global_driver->draw_pixel)
        global_driver->draw_pixel(cmd->x, cmd->y, cmd->color);
}

void display_clear_screen(const cmd_clear_screen_t* cmd){
    return;
}

int display_query_width(void){
    return 0;
}

int display_query_height(void){
    return 0;
}
