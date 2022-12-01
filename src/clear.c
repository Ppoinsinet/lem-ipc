#include <lem.h>

void clear_shm(int shm_id) {
    if (shm_id != -1)
        shmctl(shm_id, IPC_RMID, NULL);
}

void clear_ipc(t_sharedMemory *memory) {
    clear_shm(memory->game->shm_id);

    //clear display message queue
    if (memory->game->display_msg_id > 0)
        msgctl(memory->game->display_msg_id, IPC_RMID, NULL);
    unlink(FILENAME);
}