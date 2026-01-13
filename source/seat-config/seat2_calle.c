#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include "global_consts.h"
#include "global_structs.h"
#include <semaphore.h>
#include <errno.h>
#include <signal.h>

extern pid_t find_pid_by_name(const char* name);

extern const char *SOCKET_PATH;

int send_signal(int signum, const char* to) {
    pid_t found_pid;
    uint8_t retry_count = RETRY_COUNT, i = 0;
    while (i < retry_count && ((found_pid = find_pid_by_name(to)) == -1)) {
        printf("Waiting for other instance to start...\n");
        sleep(RETRY_WAIT);
        i++;
    }

    if (found_pid == -1) {
        printf("Not able to find pid for named comm\n");
        return 1;
    }

    printf("Found pid = %d for comm %.*s\n", found_pid, (int)strlen(to), to);

    kill(found_pid, signum);

    return 0;
}

// Function to receive the nested display ID
int receive_display_id(char *out_buffer, size_t buffer_size) {
    int fd;
    sem_t sem;
    struct stat st;

    sem_init(&sem, 1, 0);

    if (stat("/dev" SHM_FILE, &st) != 0 && errno != ENOENT) {
        perror("error during stat -> !ENOENT");
        send_signal(ERROR_SIGNAL, "seat1_calle");
        return 1;
    }
    else 
        shm_unlink(SHM_FILE);

    if ((fd = shm_open(SHM_FILE, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1) {
        perror("error during shm_open");
        send_signal(ERROR_SIGNAL, "seat1_calle");
        return 1;
    }

    if (ftruncate(fd, sizeof(seat_shared_ds)) != 0) {
        perror("error during ftruncate");
        send_signal(ERROR_SIGNAL, "seat1_calle");
        return 1;
    }

    seat_shared_ds_ptr sync_region = (seat_shared_ds_ptr)mmap(NULL, sizeof(seat_shared_ds), PROT_READ | PROT_WRITE, 
        MAP_SHARED, fd, 0);
    sync_region->sync_state = sem;

    printf("============= Reached checkpoint =============\n");

    printf("Sync region created successfully ...\n");

    if (send_signal(COMPL_SIGNAL, "seat1_calle") != 0) {
        printf("Unable to send signal...");
        return 1;
    }

    int server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock < 0) { 
        perror("socket"); 
        return 1; 
    }

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    // if truncated it doesn't write the null byte, so -1 for it
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    addr.sun_path[sizeof(addr.sun_path)-1] = '\0';

    unlink(SOCKET_PATH);

    if (bind(server_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) { 
        perror("bind"); 
        close(server_sock);
        return 1; 
    }

    if (listen(server_sock, 1) < 0) { 
        perror("listen"); 
        close(server_sock); 
        return 1; 
    }

    // signal other caller that it can send data
    sem_post(&sync_region->sync_state);

    printf("Waiting for display ID...\n");
    int client = accept(server_sock, NULL, NULL);
    if (client < 0) { 
        perror("accept"); 
        close(server_sock); 
        return 1; 
    }

    ssize_t n = read(client, out_buffer, buffer_size-1);
    if (n < 0) { 
        perror("read"); 
        close(client); 
        close(server_sock); 
        return 1; 
    }

    out_buffer[n] = '\0'; // null-terminate
    printf("Received display ID: %s\n", out_buffer);

    close(client);
    close(server_sock);
    close(fd);
    return 0;
}

int main() {
    // assuming for know always localhost => :X
    char display_id[32] = { [0] = ':', 0};

    setvbuf(stdout, NULL, _IONBF, 0);

    printf("Scanning for display id from other instance...\n");

    if (receive_display_id(display_id+1, sizeof(display_id)-1) != 0) {
        fprintf(stderr, "Failed to receive display ID\n");
        return 1;
    }

    printf("Starting display_manager with DISPLAY: %s\n", display_id);

    // Set the standard DISPLAY environment variable
    if (setenv("DISPLAY", display_id, 1) != 0) {
        perror("setenv failed");
        return 1;
    }

    // Prepare arguments for exec
    char *argv[] = {"/usr/local/bin/seat_display_driver/display/display", NULL};

    // Replace current process with display_manager
    execv(argv[0], argv);

    // If execv returns, something went wrong
    perror("execv failed");
    return 1;
}
