#include <lem.h>

static void child_wait_for_start(t_sharedMemory *memory, int playerIndex) {
    while (1) {
        sem_wait(memory->game->semaphore);
        if (memory->game->start == 1) {
            sem_post(memory->game->semaphore);
            player_loop(memory, playerIndex);
            exit(0);
        }
        sem_post(memory->game->semaphore);
    }
}

void child_entry(t_sharedMemory *memory) {

    signal(SIGINT, sigint_callback);

    sem_wait(memory->game->semaphore);
    for (int i = 1; i < MAX_PLAYERS; i++) {
        if (memory->game->connected[i] == 0) {
            memory->game->connected[i] = getpid();
            printf("Connected to game..\n");
            sem_post(memory->game->semaphore);

            // Entered game
            child_wait_for_start(memory, i);
            exit(0);
        }
    }
    if (memory->game->display_pid == 0) {
        printf("Game is full -- Joining as active display..\n");
        memory->game->display_pid = getpid();
        display_loop(memory);
    } else
        print_map(memory);
    sem_post(memory->game->semaphore);
    exit(0);
}