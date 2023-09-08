#include <lem.h>

void initSemaphore() {
    status.semaphore = sem_open(SEMAPHORE_NAME, O_CREAT, S_IRUSR | S_IWUSR, 1);
    if (status.semaphore == SEM_FAILED)
        onError("sem_open (initSemaphore)", 0);
}

void initPlayer() {
    sem_wait(status.semaphore);

    // Join players list
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (game->players[i].pid == 0) {
            status.playerIndex = i;
            status.msg_id = msgget(IPC_PRIVATE, 0700 | IPC_CREAT);
            if (status.msg_id == -1)
                onError("msgget", 1);

            
            break ;
        }
    }

    if (status.playerIndex < 0)
        onError("Could not add player to players list", 1);

    // Find position for player
    while (1) {
        int x = getRandomPosition(MAP_WIDTH);
        int y = getRandomPosition(MAP_HEIGHT);
        if (!getTile(x, y)) {
            status.position.x = x;
            status.position.y = y;
            game->map[x + y * MAP_WIDTH] = status.playerIndex;

            printf("Player %d : %d - %d  == %d\n", status.playerIndex, status.position.x, status.position.y, status.msg_id);
            break;
        }
    }
    game->players[status.playerIndex] = status;
    sem_post(status.semaphore);
    waitAllPlayers();
}

void initCreator() {
    shared_memory_init();

    game->players[0] = status;
    waitAllPlayers();

    status.msg_id = msgget(IPC_PRIVATE, 0700 | IPC_CREAT);
    if (status.msg_id == -1)
        onError("msgget", 1);

    // Find position for creator's player
    sem_wait(status.semaphore);

    while (1) {

        int x = getRandomPosition(MAP_WIDTH);
        int y = getRandomPosition(MAP_HEIGHT);
        if (!getTile(x, y)) {
            status.position.x = x;
            status.position.y = y;
            game->map[x + y * MAP_WIDTH] = 0;

            printf("Player %d : %d - %d  == %d\n", 0, status.position.x, status.position.y, status.msg_id);
            break;
        }
    }
    game->players[0] = status;

    sem_post(status.semaphore);
}