#include "display_driver.h"
#include "display_api.h"
#include "frame_buffer.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h> 

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/* logs message only */
static void log_message(const char *msg){
    if (msg)
        fprintf(stderr, "%s\n", msg);
}

/* logs message + pthread error */
static void log_message_with_status(const char *msg, int status) {
    if (msg)
        fprintf(stderr, "%s: %s\n", msg, strerror(status));
    else
        fprintf(stderr, "%s\n", strerror(status));
}

// logs both
void display_cb_verbose(char* msg, int status) {
    log_message_with_status(msg, status);
}

// logs only per function error message
void display_cb_log_wo(char* _, int status) {
    log_message_with_status(NULL, status);
}

// logs only message
void display_cb_log_u_only(char* msg, int _) {
    log_message(msg);
}

// logs non 
void display_cb_non(char* msg, int status) {}

void display_draw_image(const cmd_draw_image_t* cmd, display_flags_t flags, 
    display_callback_f cb, char* msg){
    int status;
    if (global_driver && global_driver->draw_image) {
        status = (flags & DISPLAY_WNO) ? 
            pthread_mutex_trylock(&lock) : 
            pthread_mutex_lock(&lock);
        if(status) {
            cb(msg);
            return;
        }
        global_driver->draw_image(
            cmd->x, cmd->y,
            cmd->width, cmd->height,
            cmd->buffer
        );
        pthread_mutex_unlock(&lock);
    }
}

void display_set_pixel(const cmd_set_pixel_t* cmd, display_flags_t flags, 
    display_callback_f cb, char* msg){
    int status;
    if (global_driver && global_driver->draw_pixel) {
        status = (flags & DISPLAY_WNO) ? 
            pthread_mutex_trylock(&lock) : 
            pthread_mutex_lock(&lock);
        if(status) {
            cb(msg);
            return;
        }
        global_driver->draw_pixel(cmd->x, cmd->y, cmd->color);
        pthread_mutex_unlock(&lock);
    }       
}

void display_clear_screen(const cmd_clear_screen_t* cmd, display_flags_t flags, 
    display_callback_f cb, char* msg){
    int status;
    if (global_driver && global_driver->clear_screen) {
        status = (flags & DISPLAY_WNO) ? 
            pthread_mutex_trylock(&lock) : 
            pthread_mutex_lock(&lock);
        if(status) {
            cb(msg);
            return;
        }
        global_driver->clear_screen(cmd->on);
        pthread_mutex_unlock(&lock);
    }
}

void display_fill_screen(const cmd_fill_screen_t* cmd, display_flags_t flags, 
    display_callback_f cb, char* msg) {
    int status;
    if (global_driver && global_driver->fill_screen) {
        status = (flags & DISPLAY_WNO) ? 
            pthread_mutex_trylock(&lock) : 
            pthread_mutex_lock(&lock);
        if(status) {
            cb(msg);
            return;
        }
        global_driver->fill_screen(cmd->color);
        pthread_mutex_unlock(&lock);
    }
}

int display_query_width(void){
    if (global_driver && global_driver->get_width)
        return global_driver->get_width();

    return -1;
}

int display_query_height(void){
    if (global_driver && global_driver->get_height)
        return global_driver->get_height();

    return -1;
}
