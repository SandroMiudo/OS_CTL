#ifndef DISPLAY_DRIVER
#define DISPLAY_DRIVER

#include "drm_driver_defs.h"
#include <xf86drmMode.h>
#include <drm/drm_mode.h>

extern uint32_t* get_connectors(drmModeRes* res, uint32_t* c);
extern uint32_t* get_encoders(drmModeRes* res, uint32_t* c);
extern uint32_t* get_controllers(drmModeRes* res, uint32_t* c);
extern drmModeConnector* get_connector(CONNECTOR_ID c, int fd);
extern drmModeEncoder* get_encoder(ENCODER_ID e, int fd);
extern drmModeCrtc* get_controller(CONTROLLER_ID c, int fd);
extern drmModeRes* getResources(int card, int* fd, 
    char* card_path, size_t path_len);
extern drmModeModeInfoPtr get_preferred_mode(drmModeConnector* conn);

extern bool is_connected(drmModeConnector* conn);

extern drmModeEncoder* bind_encoder(int fd, drmModeRes* res, drmModeConnector *conn);
extern uint8_t* register_buffer(int fd, drmModeModeInfoPtr chosen_mode, uint32_t* frame_buffer_id);
extern int connect(int fd, int crtc_id, int fb_id, drmModeModeInfoPtr chosen_mode,
    drmModeConnectorPtr conn);
extern int select_drm_card(char* out_path, size_t path_len);
extern drmModeConnector* search_for_connector(drmModeRes* resources, int fd);

extern uint32_t PROBING_SEQUENCE[];

#endif