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
    if (index < 0)
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
        printf("Player does not move\n");
        return ;
    }
    printf("Moving to %d - %d\n", player->position.x + movement.x, player->position.y + movement.y);
    movePlayerTo(player, get_coord(player->position.x + movement.x, player_position.y + movement.y));
}

void emitOrder(t_closestEnemy *target) {
    // Player calculates closest enemy
    *target = getClosestEnemy();
    if (!target->enemy) {
        return ;
    }
    // printf("Closest enemy is %d\n", target.enemy->playerIndex);
    // Player checks distance with every enemies and send instructions to subordinates if he has no instructions to follow
    t_pathFindingResult tmpDistance;
    int closestMateIndex = -1;
    int closestMateDistance = MAP_WIDTH * MAP_HEIGHT;

    // Find mate who is closest to an enemy
    for (int i = status.playerIndex + 1; i < MAX_PLAYERS; i++) {
        if (game->players[i].pid > 0 && game->players[i].team == status.team) {
            t_playerStatus *mate = &game->players[i];

            tmpDistance = getDistance(mate->position, target->enemy->position, pathFindingResult(-1, get_coord(0, 0)));
            if (tmpDistance.distance < closestMateDistance) {
                closestMateIndex = i;
                closestMateDistance = tmpDistance.distance;
            }
        }
    }

    // Return if no mates
    if (closestMateIndex == -1 || game->players[closestMateIndex].targetIndex == target->enemy->playerIndex)
        return ;
    
    // Send order
    t_orderMessagePayload msg;
    msg.distance = closestMateDistance;
    msg.enemyIndex = target->enemy->playerIndex;
    msg.providerIndex = status.playerIndex;
    if (msgsnd(game->players[closestMateIndex].msg_id, &msg, sizeof(msg), 0) == -1) {
        printf("Tried to send to %d avec %d\n", game->players[closestMateIndex].msg_id, closestMateIndex);
        perror("msgsnd (Sending order)");
    } else 
        printf("ORDERED %d TO ATTACK : %d\n", closestMateIndex, msg.enemyIndex);
}

t_closestEnemy readOrders() {
    t_closestEnemy target = {.distance = MAP_WIDTH * MAP_HEIGHT, .enemy = NULL};
    t_orderMessagePayload msg;
    ssize_t ret = 0;
    int providerIndex = -1;

    while ((ret = msgrcv(status.msg_id, &msg, sizeof(msg), 0, IPC_NOWAIT))) {

        if (ret == -1) {
            break ;

        } else if (ret > 0) {
            if (msg.enemyIndex >= 0 && (target.enemy || msg.providerIndex < providerIndex)) {
                printf("ORDER - SETTING TARGET TO : %d\n", msg.enemyIndex);
                target.enemy = &game->players[msg.enemyIndex];
                target.distance = msg.distance;
                providerIndex = msg.providerIndex;
            }
        }
    }
    return target;
}

void onQuit(int arg) {
    (void)arg;

    char isLast = 1;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (game->players[i].pid > 0 && game->players[i].pid != status.pid) {

            if (!kill(game->players[i].pid, 0)) {
                // Another process is running
                isLast = 0;
            }
        }
    }

    if (isLast) {
        printf("Cleaning all resources\n");

        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (game->players[i].pid > 0) {

                if (game->players[i].msg_id > -1) {
                    if (msgctl(game->players[i].msg_id, IPC_RMID, NULL))
                        perror("Could not close message queue");

                }
                
                if (game->players[i].shm_id > -1) {
                    if (shmctl(game->players[i].shm_id, IPC_RMID, NULL))
                        perror("Could not close semaphore");
                }
            }
        }
        unlink(FILENAME);
        if (!sem_unlink(SEMAPHORE_NAME))
            perror("sem_unlink");

    } else {
        // Is not last to quit

        if (status.msg_id > -1) {
            if (msgctl(status.msg_id, IPC_RMID, NULL))
                perror("Could not close message queue");
            else
                game->players[status.playerIndex].msg_id = -1;

        }
        
        if (status.shm_id > -1) {
            if (shmctl(status.shm_id, IPC_RMID, NULL))
                perror("Could not close semaphore");
            else
                game->players[status.playerIndex].shm_id = -1;
        }
    }
    exit(0);
}

void onDeath() {

    game->players[status.playerIndex].isDead = 1;
    if (!msgctl(status.msg_id, IPC_RMID, NULL))
        game->players[status.playerIndex].msg_id = -1;
    
    sem_post(status.semaphore);
    exit(1);
}

void onDisplayTurn() {
    if (status.playerIndex == 0)
        print_map(game->map);
    else if (!kill(game->players[0].pid, 0))
        return ;

    // Executed only by creator or if creator process has ended

    struct timeval tmp;
    gettimeofday(&tmp, NULL);

    // Next turn starts in 1 second
    game->nextTurnTimestamp = (tmp.tv_sec * 1000 * 1000) + tmp.tv_usec + (1 * 1000 * 1000);
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

        if (game->finished) {
            sem_post(status.semaphore);
            break;
        }

        if (game->turn == DISPLAY_TURN) {
            onDisplayTurn();
            sem_post(status.semaphore);
            continue;
        }

        struct timeval tmp;
        gettimeofday(&tmp, NULL);
        if (game->nextTurnTimestamp > (tmp.tv_sec * 1000 * 1000) + tmp.tv_usec) {
            // next turn did not start
            sem_post(status.semaphore);
            continue;
        }

        status = game->players[status.playerIndex];
        if (status.isDead)
            continue ;

        if (status.targetIndex > -1 && game->players[status.targetIndex].isDead)
            status.targetIndex = -1;

        // Player reads instructions and instruction from lowest teammate index
        t_closestEnemy target = readOrders();

        if (didPlayerWin()) {
            game->finished = 1;
            sem_post(status.semaphore);
            break;
        }
        // Player checks if he's dead
        if (isPlayerDead()) {
            printf("You died\n");
            game->turn = DISPLAY_TURN;

            if (status.playerIndex > 0)
                onDeath();
            else
                status.isDead = 1;
        }

        // If player did not receive any instructions
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

    printf("Congratulations, you won the game !\n");
    onQuit(0);
}