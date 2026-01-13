#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>

// relpath must be a direct succesor to rootpath
int mkdir_direct(const char *rootpath, const char *relpath, mode_t mode){
    int rootfd;
    struct stat st;

    if (strchr(relpath, '/')) {
        errno = EINVAL;
        return 1;
    }

    /* check rootpath exists and is a directory */
    if (stat(rootpath, &st) != 0 || !S_ISDIR(st.st_mode)) {
        errno = ENOTDIR;
        return 1;
    }

    /* open root directory to get dirfd */
    rootfd = open(rootpath, O_RDONLY);
    if (rootfd < 0)
        return 1;

    /* check if relpath exists under root */
    if (fstatat(rootfd, relpath, &st, 0) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            close(rootfd);
            errno = ENOTDIR;
            return 1;
        }
        close(rootfd);
        return 0;
    } 
    else if (errno != ENOENT) {
        close(rootfd);
        return 1;
    }

    /* create directory */
    int ret = mkdirat(rootfd, relpath, mode);
    close(rootfd);
    return ret;
}

pid_t find_pid_by_name(const char *name) {
    DIR *proc = opendir("/proc");
    if (!proc) {
        perror("opendir /proc failed");
        return -1;
    }

    struct dirent *entry;
    while ((entry = readdir(proc)) != NULL) {
        // Skip non-numeric entries
        if (!isdigit(entry->d_name[0]))
            continue;

        char path[256];
        snprintf(path, sizeof(path), "/proc/%.50s/comm", entry->d_name);

        FILE *f = fopen(path, "r");
        if (!f)
            continue;  // skip processes we can't read

        char comm[256];
        if (fgets(comm, sizeof(comm), f) != NULL) {
            comm[strcspn(comm, "\n")] = 0;  // remove trailing newline

            if (strcmp(comm, name) == 0) {
                fclose(f);
                closedir(proc);
                return (pid_t)atoi(entry->d_name);  // match found
            }
        }

        fclose(f);
    }

    closedir(proc);
    return -1;  // no match found
}