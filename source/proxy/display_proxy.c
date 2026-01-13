#include <systemd/sd-login.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "global_consts.h"

// Static dispatch table
struct entry {
    const char *seat_name;
    const char *binary_path;
};

static const struct entry dispatch_table[] = {
    { "Seat1", PROXY_TGT_ENTR(seat1_calle) },
    { "Seat2", PROXY_TGT_ENTR(seat2_calle) },
    { NULL, NULL } // must end with NULL
};

int main(void) {
    setvbuf(stdout, NULL, _IOLBF, 0);
    
    const char *seat = getenv("SEAT");
    if (!seat) {
        fprintf(stderr, "SEAT environment variable not set!\n");
        return 1;
    }

    // Iterate the dispatch table and call matching binaries
    for (int i = 0; dispatch_table[i].seat_name != NULL && dispatch_table[i].binary_path != NULL; i++) {
        if (strcmp(seat, dispatch_table[i].seat_name) == 0) {
            printf("Launching binary for %s: %s\n", seat, dispatch_table[i].binary_path);
            execl(dispatch_table[i].binary_path, dispatch_table[i].binary_path, NULL);

            perror("execl failed");
            return 1;
        }
    }

    fprintf(stderr, "No matching binary found for seat: %s\n", seat);
    return 1;
}
