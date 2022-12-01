#include <lem.h>

extern t_sharedMemory memory;

static void creator_init_players(t_sharedMemory *memory) {
    printf("Initializing players..\n");
    sem_wait(memory->game->semaphore);

    get_players(memory);
    printf("Players initialized\n");
    memory->game->start = 1;

    sem_post(memory->game->semaphore);
}

void creator_entry(t_sharedMemory *memory) {
    int oldCount = 0;
    memory->playerIndex = 0;

    sem_wait(memory->game->semaphore);
    if (memory->command == CMD_DISPLAY) {
        // If creator joins as display
        display_init(memory);
    } else {
        // If creator joins as player
        player_init(memory);
    }
    sem_post(memory->game->semaphore);

    while (1) {
        if (memory->die)
            client_dies(memory);

        sem_wait(memory->game->semaphore);

        int count = 0;
        
        for (int i = 0; i < MAX_PLAYERS; i++) {
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

    if (memory->command != CMD_DISPLAY)
        player_loop(memory);
    else
        display_loop(memory);
}