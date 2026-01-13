#define _GNU_SOURCE

#include <dirent.h>
#include <string.h>
#include "drm_driver_defs.h"
#include "drm_driver_core.h"
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm/drm_mode.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <drm/drm_fourcc.h>
#include <stdio.h>
#include <bits/fcntl-linux.h>
#include "drm_driver_env.h"
#include <stdlib.h>
#include "frame_buffer.h"


// --- Global DRM state (module-local static variables) ---
static int drm_fd = -1;
static uint32_t drm_frame_buffer_id = 0;
static uint8_t* drm_frame_buffer = NULL;
static drmModeRes* drm_resources = NULL;
static drmModeConnector* drm_connector = NULL;
static drmModeEncoder* drm_encoder = NULL;
static drmModeModeInfoPtr drm_mode = NULL;

// scan through /sys/ topology
void display_list_connectors_and_details() {
    const char *drm_path = "/sys/class/drm/";
    DIR *dir = opendir(drm_path);
    if (!dir) {
        perror("opendir");
        return;
    }
    struct dirent *entry;
    printf("Connectors and Details:\n");
    while ((entry = readdir(dir)) != NULL) {
        // Look for connector entries: cardX-PORTTYPE-N
        if (strncmp(entry->d_name, "card", 4) == 0 && strchr(entry->d_name, '-') != NULL) {
            // Find the card name (up to first '-')
            char card_name[32];
            const char *dash = strchr(entry->d_name, '-');
            size_t card_len = dash - entry->d_name;
            strncpy(card_name, entry->d_name, card_len);
            card_name[card_len] = '\0';
            printf("Connector: %s\n", entry->d_name);
            printf("  Belongs to card: %s\n", card_name);
            // Print connector details (read from /sys/class/drm/<connector>/)
            char conn_dir[256];
            snprintf(conn_dir, sizeof(conn_dir), "%s%s/", drm_path, entry->d_name);
            DIR *conn_info_dir = opendir(conn_dir);
            if (conn_info_dir) {
                struct dirent *info_entry;
                printf("  Details:\n");
                while ((info_entry = readdir(conn_info_dir)) != NULL) {
                    if (info_entry->d_name[0] != '.') {
                        printf("    %s\n", info_entry->d_name);
                    }
                }
                closedir(conn_info_dir);
            } else {
                printf("  (No details found)\n");
            }
        }
    }
    closedir(dir);
}

// scan throught /sys/ topology
void display_list_cards_and_ports() {
    // Scan /sys/class/drm/ for cards and their ports
    const char *drm_path = "/sys/class/drm/";
    DIR *dir = opendir(drm_path);
    if (!dir) {
        perror("opendir");
        return;
    }
    struct dirent *entry;
    int card_count = 0;
    printf("Available Display Cards and Ports:\n");
    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, "card", 4) == 0 && strchr(entry->d_name, '-') == NULL) {
            // Found a card (e.g., card0)
            card_count++;
            printf("%d. Card: %s\n", card_count, entry->d_name);
            // Now look for connectors (ports) for this card in /sys/class/drm/cardX/
            char card_dir[256];
            snprintf(card_dir, sizeof(card_dir), "%s%s/", drm_path, entry->d_name);
            DIR *conn_dir = opendir(card_dir);
            if (conn_dir) {
                struct dirent *conn_entry;
                printf("   Ports: ");
                int first = 1;
                size_t card_name_len = strlen(entry->d_name);
                while ((conn_entry = readdir(conn_dir)) != NULL) {
                    // Look for entries like cardX-PORTTYPE-N (e.g., card0-HDMI-A-1)
                    if (strncmp(conn_entry->d_name, entry->d_name, card_name_len) == 0 &&
                        conn_entry->d_name[card_name_len] == '-' &&
                        strncmp(conn_entry->d_name, ".", 1) != 0) {
                        if (!first) printf(", ");
                        // Print only the port part, e.g., DP-1 from card1-DP-1
                        printf("%s", conn_entry->d_name + card_name_len + 1);
                        first = 0;
                    }
                }
                if (first) printf("(none)");
                printf("\n");
                closedir(conn_dir);
            } else {
                // Fallback: try to list connectors in /sys/class/drm/
                DIR *drm_dir2 = opendir(drm_path);
                if (drm_dir2) {
                    struct dirent *port_entry;
                    printf("   Ports: ");
                    int first = 1;
                    while ((port_entry = readdir(drm_dir2)) != NULL) {
                        if (strncmp(port_entry->d_name, entry->d_name, strlen(entry->d_name)) == 0 &&
                            strchr(port_entry->d_name, '-') != NULL) {
                            if (!first) printf(", ");
                            printf("%s", port_entry->d_name + strlen(entry->d_name) + 1);
                            first = 0;
                        }
                    }
                    printf("\n");
                    closedir(drm_dir2);
                } else {
                    printf("   Ports: (unknown)\n");
                }
            }
        }
    }
    if (card_count == 0) {
        printf("No display cards found.\n");
    }
    closedir(dir);
}

uint32_t PROBING_SEQUENCE[] = {
    DRM_MODE_CONNECTOR_DisplayPort,
    DRM_MODE_CONNECTOR_HDMIA,
    DRM_MODE_CONNECTOR_HDMIB,
    DRM_MODE_CONNECTOR_USB,
    0
};

static const ConnMap connector_map[] = {
    { DRM_MODE_CONNECTOR_VGA, "VGA" },
    { DRM_MODE_CONNECTOR_DVII, "DVI-I" },
    { DRM_MODE_CONNECTOR_DVID, "DVI-D" },
    { DRM_MODE_CONNECTOR_DVIA, "DVI-A" },
    { DRM_MODE_CONNECTOR_Composite, "Composite" },
    { DRM_MODE_CONNECTOR_SVIDEO, "SVIDEO" },
    { DRM_MODE_CONNECTOR_LVDS, "LVDS" },
    { DRM_MODE_CONNECTOR_Component, "Component" },
    { DRM_MODE_CONNECTOR_9PinDIN, "DIN" },
    { DRM_MODE_CONNECTOR_DisplayPort, "DP" },
    { DRM_MODE_CONNECTOR_HDMIA, "HDMI-A" },
    { DRM_MODE_CONNECTOR_HDMIB, "HDMI-B" },
    { DRM_MODE_CONNECTOR_TV, "TV" },
    { DRM_MODE_CONNECTOR_eDP, "eDP" },
    { DRM_MODE_CONNECTOR_VIRTUAL, "Virtual" },
    { DRM_MODE_CONNECTOR_DSI, "DSI" },
    { 0, NULL }
};

const char* connector_type_str(uint32_t type) {
    for (int i = 0; connector_map[i].name; i++) {
        if (connector_map[i].type == type)
            return connector_map[i].name;
    }
    return "Unknown";
}

uint32_t connector_type_from_str(const char* str) {
    for (int i = 0; connector_map[i].name; i++) {
        if (strcmp(connector_map[i].name, str) == 0)
            return connector_map[i].type;
    }
    return 0;
}

// Inline function to calculate the length of the probing sequence
static inline size_t probing_sequence_len(void) {
    size_t len = 0;
    while (PROBING_SEQUENCE[len] != 0) {
        len++;
    }
    return len;
}

// Function to display the current probing sequence
void show_probing_sequence(void) {
    printf("Probing sequence:\n");
    for (size_t i = 0; i < probing_sequence_len(); i++) {
        printf("[%zu]: %u\n", i, PROBING_SEQUENCE[i]);
    }
}

// Function to swap/change two entries in the probing sequence
int modify_probing_sequence(size_t idx1, size_t idx2) {
    size_t len = probing_sequence_len();

    if (idx1 >= len || idx2 >= len) {
        return -1; // invalid indices
    }

    uint32_t tmp = PROBING_SEQUENCE[idx1];
    PROBING_SEQUENCE[idx1] = PROBING_SEQUENCE[idx2];
    PROBING_SEQUENCE[idx2] = tmp;

    return 0; // success
}

inline uint32_t* get_connectors(drmModeRes* res, uint32_t* c) {
    *c = res->count_connectors;
    return res->connectors;
}

inline uint32_t* get_encoders(drmModeRes* res, uint32_t* c) {
    *c = res->count_encoders;
    return res->encoders;
}

inline uint32_t* get_controllers(drmModeRes* res, uint32_t* c) {
    *c = res->count_crtcs;
    return res->crtcs;
}

inline drmModeConnector* get_connector(CONNECTOR_ID c, int fd) {
    return drmModeGetConnector(fd, c);
}

inline bool is_connected(drmModeConnector* conn) {
    return conn->connection == DRM_MODE_CONNECTED;
}

inline bool is_connector_type(drmModeConnector* conn, uint32_t type) {
    return conn->connector_type == type;
}

inline bool has_connectors(drmModeResPtr res) {
    return res->count_connectors > 0;
}

inline drmModeEncoder* get_encoder(ENCODER_ID e, int fd) {
    return drmModeGetEncoder(fd, e);
}

inline drmModeCrtc* get_controller(CONTROLLER_ID c, int fd) {
    return drmModeGetCrtc(fd, c);
}

inline drmModeModeInfo* get_mode(drmModeConnector* conn, uint32_t idx) {
    if (idx >= conn->count_modes)
        return NULL;
    
    return &conn->modes[idx];
}

inline drmModeRes* getResources(int card, int* fd, 
    char* card_path, size_t path_len) {
    char buffer[MAX_CARD_PATH];
    snprintf(buffer, sizeof(buffer), "/dev/dri/card%d", card);
    int fd = open(buffer, O_RDWR | O_CLOEXEC);

    if (fd < 0) {
        perror("open");
        return NULL;
    }

    *fd = fd;
    strncpy(card_path, buffer, path_len);

    return drmModeGetResources(fd);
}

inline drmModeModeInfoPtr get_preferred_mode(drmModeConnector* conn) {
    if (!conn || conn->count_modes == 0)
        return NULL;

    for (int i = 0; i < conn->count_modes; i++) {
        if (conn->modes[i].type & DRM_MODE_TYPE_PREFERRED) {
            return &conn->modes[i];
        }
    }

    // fallback: return first mode if no preferred mode found
    return &conn->modes[0];
}

void display_connector_allowed_modes(drmModeConnector* conn) {
    if (!conn) {
        perror("Connector is NULL");
        return;
    }

    printf("Connector %u supports %d mode(s):\n", conn->connector_id, conn->count_modes);

    for (int i = 0; i < conn->count_modes; i++) {
        drmModeModeInfo *mode = &conn->modes[i];
        printf("  Mode %d: %s, %dx%d @ %dHz\n",
               i,
               mode->name,
               mode->hdisplay,
               mode->vdisplay,
               mode->vrefresh);
    }
}

/*// Map connector type enum to string
const char* connector_type_str(uint32_t type) {
    switch (type) {
        case DRM_MODE_CONNECTOR_Unknown:   return "Unknown";
        case DRM_MODE_CONNECTOR_VGA:       return "VGA";
        case DRM_MODE_CONNECTOR_DVII:      return "DVI-I";
        case DRM_MODE_CONNECTOR_DVID:      return "DVI-D";
        case DRM_MODE_CONNECTOR_DVIA:      return "DVI-A";
        case DRM_MODE_CONNECTOR_Composite: return "Composite";
        case DRM_MODE_CONNECTOR_SVIDEO:    return "S-Video";
        case DRM_MODE_CONNECTOR_LVDS:      return "LVDS";
        case DRM_MODE_CONNECTOR_Component: return "Component";
        case DRM_MODE_CONNECTOR_9PinDIN:   return "DIN-9";
        case DRM_MODE_CONNECTOR_DisplayPort: return "DisplayPort";
        case DRM_MODE_CONNECTOR_HDMIA:     return "HDMI-A";
        case DRM_MODE_CONNECTOR_HDMIB:     return "HDMI-B";
        case DRM_MODE_CONNECTOR_TV:        return "TV";
        case DRM_MODE_CONNECTOR_eDP:       return "eDP";
        case DRM_MODE_CONNECTOR_VIRTUAL:   return "Virtual";
        case DRM_MODE_CONNECTOR_DSI:       return "DSI";
        case DRM_MODE_CONNECTOR_DPI:       return "DPI";
        default: return "Other";
    }
} */

// Display connector information
void display_connector(drmModeConnector *conn) {
    if (!conn) {
        printf("Invalid connector\n");
        return;
    }

    printf("Connector ID: %u\n", conn->connector_id);
    printf("Connector Type: %s (%u)\n",
           connector_type_str(conn->connector_type),
           conn->connector_type);
    printf("Connector Type ID: %u\n", conn->connector_type_id);

    // Connection status
    switch (conn->connection) {
        case DRM_MODE_CONNECTED:
            printf("Connection: connected\n");
            break;
        case DRM_MODE_DISCONNECTED:
            printf("Connection: disconnected\n");
            break;
        case DRM_MODE_UNKNOWNCONNECTION:
        default:
            printf("Connection: unknown\n");
            break;
    }
}

void extract_numbers(const char* input, char* output) {
    size_t j = 0;
    for (size_t i = 0; input[i] != '\0'; i++) {
        if (isdigit((unsigned char)input[i])) {
            output[j++] = input[i];
        }
    }
    output[j] = '\0';
}

int probe_card(char* card_str, char* out_path, size_t path_len) {
    int fd = -1;

    extract_numbers(card_str, card_str);
    int cardN = (int)strtol(card_str, NULL, 10);
    drmModeResPtr res = getResources(cardN, &fd, out_path, path_len);

    if (res == (void*)0)
        goto close_routine;

    if (!has_connectors(res))
        goto close_routine;
    
    close_routine:
        if (fd >= 0) close(fd);
        if (res != NULL) drmModeFreeResources(res);
        return -1;
    success_routine:
        drmModeFreeResources(res);
        return fd;
}

drmModeConnector* probe_connector(uint32_t* connectors, int connector_type, int fd, int count) {
    for (int j = 0; j < count; j++) {
        drmModeConnector* c = get_connector(connectors[j], fd);
        if (!c) continue;

        if (is_connector_type(c, connector_type) && is_connected(c)) {
            printf("Using connector from probing sequence: %s (id=%u)\n",
                    connector_type_str(connector_type), c->connector_id);
            return c;
        }
        drmModeFreeConnector(c);
    }

    return NULL;
}

drmModeConnector* search_for_connector(drmModeRes* resources, int fd) {
    // 1. Check env var
    const char* env = getenv(DRM_CONN);
    uint32_t count = 0;
    uint32_t* connectors = get_connectors(resources, &count);
    drmModeConnector* drmConn = NULL;
    if (env) {
        uint32_t env_type = connector_type_from_str(env);
        if (env_type != 0) {
            if ((drmConn = drm_connprobe_connector(connectors, env_type, fd, count)) != NULL)
                return drmConn;    
            printf("DRM_CONN=%s but no connected connector of that type found.\n", env);
        } 
        else
            printf("Invalid DRM_CONN value: %s\n", env);
    }

    // 2. Fallback to probing sequence
    for (size_t i = 0; PROBING_SEQUENCE[i] != 0; i++) {
        uint32_t probe_type = PROBING_SEQUENCE[i];

        if ((drmConn = probe_connector(connectors, probe_type, fd, count)) != NULL)
            return drmConn;
    }

    printf("No valid connectors found.\n");
    return NULL;
}

// Returns the open fd of the selected card, or -1 on failure
int select_drm_card(char* out_path, size_t path_len) {
    int fd = -1;
    char* env_card = getenv(DRM_CARD);
    char* env_num  = getenv(DRM_NUM);
    char card_path[MAX_CARD_PATH];

    // 1. Try DRM_CARD
    if (env_card && strlen(env_card) < MAX_CARD_PATH) {
        fd = probe_card(env_card, out_path, path_len);
        if (fd >= 0)
            return fd;
    }

    // 2. Try DRM_NUM
    if (env_num && strlen(env_num) < MAX_CARD_PATH) {
        fd = probe_card(env_num, out_path, path_len);
        if (fd >= 0)
            return fd;
    }

    // 3. Enumerate /dev/dri/card*
    DIR* dir = opendir("/dev/dri");
    if (!dir) return -1;

    struct dirent* entry;
    while ((entry = readdir(dir))) {
        if (strncmp(entry->d_name, "card", 4) != 0) continue;
    
        fd = probe_card(entry->d_name, out_path, path_len);
        if (fd < 0) continue;

        closedir(dir);
        return fd;
    }

    closedir(dir);
    return -1; // No valid card found
}

drmModeEncoder* bind_encoder(int fd, drmModeRes* res, drmModeConnector *conn) {
    if (!conn || conn->count_encoders == 0) {
        fprintf(stderr, "No encoders available for connector %u\n", conn ? conn->connector_id : 0);
        return NULL;
    }

    drmModeEncoder *enc = NULL;

    if (conn->encoder_id != 0)
        enc = drmModeGetEncoder(fd, conn->encoder_id);
    else 
        enc = drmModeGetEncoder(fd, conn->encoders[0]);

    if (!enc) {
        perror("drmModeGetEncoder failed");
        return NULL;
    }

    for (int i = 0; i < res->count_crtcs; i++) {
        if (enc->possible_crtcs & (1 << i)) {
            enc->crtc_id = res->crtcs[i];
            conn->encoder_id = enc->encoder_id;
            break;
        }
    }

    if (enc->crtc_id == 0) {
        fprintf(stderr, "No compatible CRTC found for encoder %u\n", enc->encoder_id);
        drmModeFreeEncoder(enc);

        enc = NULL;
    }

    return enc;
}

uint8_t* register_buffer(int fd, drmModeModeInfoPtr chosen_mode, uint32_t* frame_buffer_id) {
    CREQ_ARGB(chosen_mode->hdisplay, chosen_mode->vdisplay);

    if (drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq) < 0) {
        perror("DRM_IOCTL_MODE_CREATE_DUMB");
        return NULL;
    }

    uint32_t fb_id;
    CMD_ARGB(creq);

    if (drmIoctl(fd, DRM_IOCTL_MODE_ADDFB2, &cmd) < 0) {
        perror("DRM_IOCTL_MODE_ADDFB2");
        return NULL;
    }

    fb_id = cmd.fb_id;

    struct drm_mode_map_dumb mreq = {0};
    mreq.handle = creq.handle;

    if (drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq) < 0) {
        perror("DRM_IOCTL_MODE_MAP_DUMB");
        return NULL;
    }

    uint8_t *fb_ptr = (uint8_t*)mmap(0, creq.size,
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED,
                        fd, mreq.offset);

    if (fb_ptr == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }

    memset(fb_ptr, 0xFF, creq.size); // make screen white

    *frame_buffer_id = fb_id;

    return fb_ptr;
}

int connect(int fd, int crtc_id, int fb_id, drmModeModeInfoPtr chosen_mode,
    drmModeConnectorPtr conn) {
    return drmModeSetCrtc(fd,
               crtc_id,           // controller
               fb_id,             // framebuffer
               0, 0,              // x,y offset in buffer
               &conn->connector_id, 1,
               chosen_mode);     // resolution/mode
}



// --- DRM init routine ---
uint8_t drm_driver_init(void) {
    char buffer[MAX_CARD_PATH];

    setvbuf(stdout, NULL, _IOLBF, 0);
    printf("Using DRM Driver ...\n");

    drm_fd = select_drm_card(buffer, sizeof(buffer));
    if (drm_fd == -1) {
        fprintf(stderr, "No suitable DRM card found\n");
        return 0;
    }

    drm_resources = drmModeGetResources(drm_fd);
    drm_connector = search_for_connector(drm_resources, drm_fd);
    if (!drm_connector) return 0;

    drm_encoder = bind_encoder(drm_fd, drm_resources, drm_connector);
    if (!drm_encoder) return 0;

    drm_mode = get_preferred_mode(drm_connector);
    if (!drm_mode) return 0;

    printf("Preferred mode: %s %ux%u @ %u Hz\n",
           drm_mode->name, drm_mode->hdisplay, drm_mode->vdisplay, drm_mode->vrefresh);

    drm_frame_buffer = register_buffer(drm_fd, drm_mode, &drm_frame_buffer_id);
    if (!drm_frame_buffer) return 0;

    if (!connect(drm_fd, drm_encoder->crtc_id, drm_frame_buffer_id, drm_mode, drm_connector))
        return 0;

    return 1; // Success
}

void drm_run() {
    
}