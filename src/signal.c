#include <lem.h>

extern t_sharedMemory memory;

void display_sigint_callback(int arg) {
    printf("\tDisplay SIGINT callback\n");
    (void)arg;
    
    sem_wait(memory.game->semaphore);

    memory.game->display_pid = 0;

    if (is_last_client(&memory)) {
        // Is last client connected -> Has to clear SharedMemory
        clear_ipc(&memory);
        sem_post(memory.game->semaphore);
        sem_unlink(SEMAPHORE_NAME);
        exit(0);
    }

    sem_post(memory.game->semaphore);
    exit(0);
}

void player_sigint_callback(int arg) {
    printf("\tPlayer SIGINT callback\n");
    (void)arg;
    memory.die = 1;
}