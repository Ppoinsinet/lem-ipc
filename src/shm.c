#include <lem.h>

void *attachSharedMemory(int shm_id) {
    void *result = shmat(shm_id, NULL, 0);
    if (result == NULL)
        onError("shmat", 0);
    printf("Got shared memory\n");
    return result;
}

/**
 * @brief Fetch or create game data (shared memory)
 * 
 * @return t_gameData* 
 */
t_gameData *openGame() {
    int rd = -2;

    if (access(FILENAME, F_OK)) {
        // file doesn't exist
        printf("Trying to create game..\n");
        rd = open(FILENAME, O_CREAT, 0666);
        if (rd == -1) {
            perror("open (2)");
            exit(-5);
        }

        // Define player as creator of game
        status.playerIndex = 0;
    }
    
    key_t key = ftok(FILENAME, 0);
    if (key == IPC_RESULT_ERROR)
        onError("ftok", 0);
    
    if (rd > 1)
        close(rd);

    if (!status.playerIndex) {
        // Creator
        status.shm_id = shmget(key, sizeof(t_gameData), 0666 | IPC_EXCL | IPC_CREAT);
    } else {
        // Player
        status.shm_id = shmget(key, sizeof(t_gameData), 0666);
    }

    if (status.shm_id == -1)
        onError("shmget", 0);

    return attachSharedMemory(status.shm_id);
}



/**
 * @brief Counts all players and returns when n_players >= MIN_PLAYERS
 * 
 */
void waitAllPlayers() {
    while (1) {
        sem_wait(status.semaphore);
        if (game->start)
            break;

        game->n_players = 0;
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (game->players[i].pid > 0) {
                // Index is occupied by player
                
                if (!kill(game->players[i].pid, 0)) {
                    // If process is not dead
                    game->n_players++;
                } else {
                    // Player disconnected while waiting for start -> free space
                    game->players[i].pid = 0;
                }
            }
        }

        printf("%d players are connected\n", game->n_players);
        if (game->n_players >= MIN_PLAYERS) {
            // Enough players are connected -> start game
            printf("Starting game with %d players\n", game->n_players);
            game->start = 1;
        }
        sem_post(status.semaphore);
        sleep(2);
    }
    sem_post(status.semaphore);
}

/**
 * @brief Executed by creator
 * 
 */
void shared_memory_init() {
    sem_wait(status.semaphore);
    game->start = 0;
    game->n_players = 1;
    game->turn = DISPLAY_TURN;
    game->finished = 0;
    game->nextTurnTimestamp = 0;

    for (int i = 0; i < MAP_WIDTH * MAP_HEIGHT; i++) {
        game->map[i] = -1;
    }

    game->players[0].pid = status.pid;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        game->players[i].pid = 0;
    }
    sem_post(status.semaphore);
}