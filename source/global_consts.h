#ifndef GLOBAL_CONSTS
#define GLOBAL_CONSTS

#include <signal.h>

#define ROOT_DIR "/usr/local/bin/seat_display_driver/"

#define PROXY_DIR ROOT_DIR "proxy/"

#define PROXY_TGT_DIR PROXY_DIR "target/"

#define PROXY_TGT_ENTR(callee) PROXY_TGT_DIR #callee

#define RUN_DIR "/run/"

#define RUN_TGT_FILE(delim) "seat_display_driver" delim

#define RUN_TGT_DIR RUN_DIR RUN_TGT_FILE("/")

#define MERGE(dir, entry) dir #entry

#define MERGE_ENTRY(entry) #entry

#define MERGE_WITH_I(dir, entry) dir MERGE_ENTRY(entry)

#define DISPLAY_DRIVER_FILE display_id
#define DISPLAY_SYNC_FILE snyc_display

#define SHM_FILE "/display_driver_shm"

#define RETRY_COUNT 5
#define RETRY_WAIT 1

#define ERROR_SIGNAL SIGUSR1
#define COMPL_SIGNAL SIGUSR2

#endif