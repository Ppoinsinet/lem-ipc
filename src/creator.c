#include <lem.h>

extern t_sharedMemory memory;

static void creator_init_players(t_sharedMemory *memory) {
    printf("Initializing players..\n");
    sem_wait(memory->game->semaphore);

    get_players(memory, memory->msg_ids);
    printf("Players initialized\n");
    memory->game->start = 1;

    sem_post(memory->game->semaphore);
}

void creator_entry(t_sharedMemory *memory) {
    int oldCount = 0;

    signal(SIGINT, sigint_callback);

    while (1) {
        sem_wait(memory->game->semaphore);

        int count = 1;
        
        for (int i = 1; i < MAX_PLAYERS; i++) {
            if (memory->game->connected[i])
                count++;
        }
        if (count != oldCount) {
            printf("%d players connected\n", count);
            memory->game->n_players = count;
            oldCount = count;
        }
        sem_post(memory->game->semaphore);
        if (count == MAX_PLAYERS)
            break ;
    }

    creator_init_players(memory);
    player_loop(memory, 0);
}