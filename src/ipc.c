#include <lem.h>

void clear_msg(int *msg_ids, int size) {
    for (int i = 0; i < size; i++) {
        
        if (msg_ids[i] != -1 && msg_ids[i]) {
            // printf("Clearing message queue %d\n", msg_ids[i]);
            msgctl(msg_ids[i], IPC_RMID, NULL);
        }
    }
}

void clear_shm(int shm_id) {
    if (shm_id != -1)
        shmctl(shm_id, IPC_RMID, NULL);
}

void clear_ipc(t_sharedMemory *memory) {
    clear_shm(memory->game->shm_id);
    clear_msg(memory->msg_ids, MAX_PLAYERS);
    unlink(FILENAME);
}

