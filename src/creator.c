#include <lem.h>

extern t_sharedMemory memory;

static void creator_start_game(t_sharedMemory *memory) {
    sem_wait(memory->game->semaphore);

    printf("Starting game\n");
    memory->game->start = 1;

    sem_post(memory->game->semaphore);
}

void creator_entry(t_sharedMemory *memory) {
    int oldCount = 0;
    memory->playerIndex = 0;

    sem_wait(memory->game->semaphore);
    for (int i = 0; i < MAP_WIDTH * MAP_HEIGHT; i++) {
        memory->game->map[i].playerIndex = -1;
        memory->game->map[i].team = 0;
    }

    signal(SIGINT, player_sigint_callback);
    if (memory->command != CMD_DISPLAY) {
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
        if (count >= MIN_PLAYERS)
            break ;
    }

    creator_start_game(memory);

    if (memory->command != CMD_DISPLAY)
        player_loop(memory);
    else {
        display_init(memory);
        display_loop(memory);
    }
}