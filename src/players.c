#include <lem.h>

t_closestEnemy getClosestEnemy() {
    t_playerStatus *player = &game->players[status.playerIndex];
    t_closestEnemy ret = {.distance = MAP_WIDTH * MAP_HEIGHT, .enemy = NULL};


    for (int i = 0; i < MAX_PLAYERS; i++) {

        if (i == status.playerIndex)
            continue;

        if (game->players[i].pid > 0 && game->players[i].team != player->team && game->players[i].isDead == 0) {
            // If enemy
            t_playerStatus *enemy = &game->players[i];
            t_pathFindingResult tmp = getDistance(player->position, enemy->position, pathFindingResult(-1, get_coord(0,0)));
            if (ret.distance > tmp.distance) {
                ret.distance = tmp.distance;
                ret.enemy = enemy;
            }
        }
    }
    return ret;
}

t_playerStatus *getTile(int x, int y) {
    int index = game->map[x + y * MAP_WIDTH];
    if (index < 0 || game->players[index].isDead)
        return NULL;
    return &game->players[index];
}

char isPlayerDead() {
    int count = 0;

    if (status.position.x > 0) {
        t_playerStatus *tmp = getTile(status.position.x - 1, status.position.y);
        if (tmp) {
            if (tmp->team != status.team)
                count++;
        }
    }
    if (status.position.x < MAP_WIDTH - 1) {
        t_playerStatus *tmp = getTile(status.position.x + 1, status.position.y);
        if (tmp) {
            if (tmp->team != status.team)
                count++;
        }
    }
    if (status.position.y > 0) {
        t_playerStatus *tmp = getTile(status.position.x, status.position.y - 1);
        if (tmp) {
            if (tmp->team != status.team)
                count++;
        }
    }
    if (status.position.y < MAP_HEIGHT - 1) {
        t_playerStatus *tmp = getTile(status.position.x, status.position.y + 1);
        if (tmp) {
            if (tmp->team != status.team)
                count++;
        }
    }
    return (char)count >= 2;
}



void movePlayerTo(t_playerStatus *player, t_coord coord) {
    // Refresh map
    // Remove old coords
    game->map[player->position.x + MAP_WIDTH * player->position.y] = -1;

    // Add new coords
    game->map[coord.x + MAP_WIDTH * coord.y] = status.playerIndex;

    status.position.x = coord.x;
    status.position.y = coord.y;
    
}

void moveTowardsEnemy(t_playerStatus *player, t_playerStatus *enemy) {

    t_coord movement = pathFinding(*player, *enemy);
    t_coord player_position = player->position;
    printf("Player position : %d - %d\nEnemy position : %d - %d\n", player->position.x, player->position.y, enemy->position.x, enemy->position.y);

    if (!movement.x && !movement.y) {
        printf("Player does not move\n-----\n");
        return ;
    }
    printf("Moving to %d - %d\n-----\n", player->position.x + movement.x, player->position.y + movement.y);
    movePlayerTo(player, get_coord(player->position.x + movement.x, player_position.y + movement.y));
}

void onLastPlayerLeaving() {
    char isLast = 1;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (game->players[i].pid > 0 && game->players[i].pid != status.pid) {

            if (!kill(game->players[i].pid, 0)) {
                // Another process is running
                isLast = 0;
                break;
            }
        }
    }

    sem_post(status.semaphore);

    if (!isLast)
        return ;
    printf("Last player leaving. Cleaning resources..\n");
    if (unlink(FILENAME))
        perror("unlink");
    if (sem_unlink(SEMAPHORE_NAME))
        perror("sem_unlink");
}

void onDeath() {
    game->players[status.playerIndex].isDead = 1;
    if (!closeMessageQueue(status.msg_id))
        game->players[status.playerIndex].msg_id = -1;

    if (status.playerIndex == 0) {
        // Creator
        status.isDead = 1;
        onLastPlayerLeaving();

    } else {

        game->players[status.playerIndex].pid = 0;
        closeSharedMemory(status.shm_id);
        onLastPlayerLeaving();
        exit(0);
    }
    
}

void onDisplayTurn() {
    if (status.playerIndex == 0)
        print_map(game->map);
    else if (!kill(game->players[0].pid, 0))
        return ;

    for (int i = 1; i < MAX_PLAYERS; i++) {
        t_playerStatus person = game->players[i];
        if (person.pid > 0 && kill(person.pid, 0)) {
            // If player is dead
            game->players[i].pid = 0;
        }
    }

    // Executed only by creator or if creator process has ended
    struct timeval tmp;
    gettimeofday(&tmp, NULL);

    // Next turn starts in 1 second
    game->nextTurnTimestamp = ((tmp.tv_sec + 1) * 1000 * 1000) + tmp.tv_usec;
    game->turn = PLAYER_TURN;
}

char didPlayerWin() {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (game->players[i].team != status.team && game->players[i].isDead == 0 && game->players[i].pid > 0)
            return 0;
    }
    return 1;
}

void playerLoop() {
    // Game starts

    while (1) {
        
        sem_wait(status.semaphore);

        if (game->finished)
            break;

        if (game->turn == DISPLAY_TURN) {
            onDisplayTurn();
            sem_post(status.semaphore);
            continue;
        }

        struct timeval tmp;
        gettimeofday(&tmp, NULL);
        long currentTime = (tmp.tv_sec * 1000 * 1000) + tmp.tv_usec;

        if (game->nextTurnTimestamp > currentTime) {
            // next turn did not start
            sem_post(status.semaphore);
            continue;
        }

        status = game->players[status.playerIndex];
        if (status.isDead) {
            sem_post(status.semaphore);
            continue ;
        }

        // If target is already dead -> reset target
        if (status.targetIndex > -1 && game->players[status.targetIndex].isDead)
            status.targetIndex = -1;
        
        // Player reads instructions and instruction from lowest teammate index
        t_closestEnemy target = readOrders();

        if (didPlayerWin()) {
            game->finished = status.team;
            sem_post(status.semaphore);
            continue;
        }
        // Player checks if he's dead
        if (!game->players[status.playerIndex].isDead && isPlayerDead()) {
            printf("You died\n");
            game->turn = DISPLAY_TURN;

            onDeath();
            continue ;
        }

        // If player did not receive any instructions
        if (!target.enemy)
            emitOrder(&target);

        // Player plays
        if (target.enemy) {
            moveTowardsEnemy(&game->players[status.playerIndex], target.enemy);
        }
        
        status.targetIndex = target.enemy ? target.enemy->playerIndex : -1;
        game->players[status.playerIndex] = status;
        game->turn = DISPLAY_TURN;
        sem_post(status.semaphore);
    }

    game->players[status.playerIndex].pid = -1;
    if (game->finished == status.team)
        printf("Congratulations, you won the game !\n");
    else
        printf("Too bad, you lost the game..\n");

    onDeath();
}