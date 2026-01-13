#include "display_driver.h"
#include <stdio.h>
#include <stdlib.h>
#include <err.h>

extern char **environ;

int main(void) {
    setvbuf(stdout, NULL, _IOLBF, 0);

    char* display = getenv("DISPLAY");
    char* xauth   = getenv("XAUTHORITY");

    if (display == NULL)
        err(EXIT_FAILURE, "Display ID is not set, but is required for the x client");

    if (xauth == NULL)
        err(EXIT_FAILURE, "Xauthority is not set, but is required for the x client");

    printf("Entered display manager : Targeting Display ID => %s\n", display);
    printf("Using %s for authentication\n", xauth);

    if (global_driver == NULL) {
        fprintf(stderr, "No display driver selected at compile time.\n");
        return 1;
    }

    printf("=========== ENV Sec ===========\n");

    for (char **e = environ; *e; e++)
        fprintf(stdout, "|%10s\n |", *e);

    printf("=========== ******* ===========\n");

    // Initialize the driver

    printf("========== Starting init routine ==========\n");

    uint8_t status = global_driver->init_routine();
    if (status != 0) {
        fprintf(stderr, "Display driver initialization failed (code %u).\n", status);
        return 1;
    }

    printf("========== Starting run routine ==========\n");

    // Run the driver
    global_driver->run();

    return 0;
}
