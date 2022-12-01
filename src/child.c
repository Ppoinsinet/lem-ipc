#include <lem.h>

static void child_wait_for_start(t_sharedMemory *memory) {
    while (1) {
        if (memory->die)
            client_dies(memory);
        sem_wait(memory->game->semaphore);
        if (memory->game->start == 1) {
            sem_post(memory->game->semaphore);
            player_loop(memory);
            exit(0);
        }
        sem_post(memory->game->semaphore);
    }
}

void child_entry(t_sharedMemory *memory) {
    printf("Child entry..\n");

    if (memory->command != CMD_DISPLAY) {
        // Joins as player
        sem_wait(memory->game->semaphore);
        int ret = player_init(memory);
        sem_post(memory->game->semaphore);
        if (ret) {
            exit(1);
        }

        // Wait for game to be full
        child_wait_for_start(memory);
    } else {
        // Joins as display
        sem_wait(memory->game->semaphore);

        if (display_init(memory)) {
            // Active display is already running -- Printing map
            sem_post(memory->game->semaphore);
            exit(0);
        } else {
            // Active display is not running -- Going in display loop
            sem_post(memory->game->semaphore);
            display_loop(memory);
        }
    }
}