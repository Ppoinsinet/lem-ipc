#include <lem.h>

extern t_sharedMemory memory;

void sigint_callback(int arg) {
    printf("\tSIGINT callback\n");
    (void)arg;

    sem_wait(memory.game->semaphore);
    printf("Got semaphore\n");

    int pid = getpid();
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (memory.game->connected[i] == pid) {
            player_dies(&memory, i);
            exit(0);
        }
    }
    sem_post(memory.game->semaphore);
    printf("Ciao\n");
    exit(0);
}