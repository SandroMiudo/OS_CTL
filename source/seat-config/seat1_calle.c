#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>
#include "global_consts.h"
#include "global_structs.h"
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>

extern uint32_t get_usable_gpu_count(void);
extern int mkdir_direct(const char* rootpath, const char* relpath, mode_t mode);

extern const char* SOCKET_PATH;

int send_display_id(const char *display_id) {
    int fd;
    if (!display_id) {
        fprintf(stderr, "NESTED_DISPLAY not set!\n");
        return 1;
    }

    // sync signal handling routine (completion based approach)
    sigset_t set;
    int sig;

    // USR1 == error , USR2 == continue

    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGUSR2);

    // Block SIGUSR1 in this thread
    sigprocmask(SIG_BLOCK, &set, NULL);

    printf("Waiting for SIGUSR1 or SIGUSR2...\n");

    if (sigwait(&set, &sig) != 0) {
        perror("sigwait failed");
        return 1;
    }

    if (sig == ERROR_SIGNAL) {
        printf("Error occured in other instance, probably \
             while creating shared memory object");
        return 1;
    }

    if ((fd = shm_open(SHM_FILE, O_RDWR, 0)) == -1) {
        perror("error during shm_open");
        return 1;
    }

    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) { 
        perror("socket"); 
        return 1; 
    }

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), SOCKET_PATH);

    // sync with other instance
    seat_shared_ds_ptr sync_region = (seat_shared_ds_ptr)mmap(NULL, sizeof(seat_shared_ds), PROT_READ | PROT_WRITE, 
        MAP_SHARED, fd, 0);

    if (sync_region == MAP_FAILED) {
        perror("error during mmap");
        return 1;
    }

    printf("Sync with other instace ...\n");
    
    if (sem_wait(&sync_region->sync_state) != 0) {
        perror("error during sem_wait");
        return 1;
    }

    printf("Sync completed ...\n");

    // Connect to receiver
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }

    if (write(sock, display_id, strlen(display_id)) == -1) {
        perror("error during write");
        return 1;
    }
    close(sock);

    printf("Sent nested display ID: %s\n", display_id);
    return 0;
}

int main() {
    int ret, n;
    char buf[32] = 
        { 0, [31] = '\0' };

    setvbuf(stdout, NULL, _IOLBF, 0);

    unsigned int gpu_count = get_usable_gpu_count();
    printf("Detected %u usable GPU(s)\n", gpu_count);
    if (gpu_count == 1) {
        if (mkdir_direct(RUN_DIR, RUN_TGT_FILE(""), S_IWUSR | S_IRUSR) != 0) {
            perror("error occured during mkdir_direct");
            return 1;
        }

        int fd = open(MERGE_WITH_I(RUN_TGT_DIR, DISPLAY_DRIVER_FILE), O_RDWR | O_CREAT, 
            S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (fd == -1) {
            perror("error occured during open");
            return 1;
        }
        ret = ftruncate(fd, sizeof(buf));
        if (ret != 0) {
            fprintf(stderr, "Failed to truncate to given size\n");
            return 1;
        }
        
        printf("Only 1 GPU detected, starting nested compositor...\n");
        ret = system(MERGE(ROOT_DIR, start_nested_x));
        if (ret != 0) {
            fprintf(stderr, "Failed to execute start_nested_x.sh\n");
            return 1;
        }

        char* display_id = (char*)mmap(NULL, sizeof(buf), PROT_READ | PROT_WRITE, MAP_PRIVATE,
            fd, 0);
            
        if (display_id == MAP_FAILED) {
            perror("error occured during mmap");
            return 1;
        }
        display_id[31] = '\0';

        uint64_t converted_display_id = strtoull(display_id, NULL, 10);

        printf("Display ID mmap : %ld\n", converted_display_id);

        if ((n = snprintf(buf, sizeof(buf), "%ld", converted_display_id)) < 0) {
            printf("error occured during snprintf");
            return 1;
        }

        printf("============= Reached checkpoint =============\n");
        printf("Sending display %2$.*1$s id to recipent ...\n", n, buf);

        // Only call send_display_id() if nested X started successfully
        if (send_display_id(buf) != 0) {
            fprintf(stderr, "Failed to send display ID\n");
            return 1;
        }
    } 
    else {
        printf("Multiple GPUs detected, no nested compositor needed.\n");
        // handle multi-GPU logic here, use kmscon to start a drm based terminal on second card
    }

    return 0;
}