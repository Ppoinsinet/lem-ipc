#include <lem.h>

char closeMessageQueue(int id) {
    if (id <= 0)
        return 0;
    if (msgctl(id, IPC_RMID, NULL)) {
        perror("Could not close message queue");
        return 1;
    }
    return 0;
}

char closeSharedMemory(int id) {
    if (id <= 0)
        return 0;
    if (shmctl(status.shm_id, IPC_RMID, NULL)) {
        perror("Could not close semaphore");
        return 1;
    }
    return 0;
}