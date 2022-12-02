#include <lem.h>

extern t_sharedMemory memory;

int player_init(t_sharedMemory *memory) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (memory->game->connected[i] == 0) {
            memory->game->players[i].msg_id = msgget(IPC_PRIVATE, 0700 | IPC_CREAT);
            if (memory->game->players[i].msg_id == -1) {
                perror("msgget");
                return 3;
            }
            printf("Player %d setup %d message queue\n", memory->playerIndex, memory->game->players[i].msg_id);
            memory->game->players[i].playerIndex = i;

            if (memory->command == CMD_TEAM_1)
                memory->game->players[i].team = 1;
            else if (memory->command == CMD_TEAM_2)
                memory->game->players[i].team = 2;
            else {
                printf("Error player initialization -- command was %d\n", memory->command);
                return 2;
            }

            while (1) {
                int x = getRandomPosition(MAP_WIDTH);
                int y = getRandomPosition(MAP_HEIGHT);
                if (!player_on_tile(memory->game, x, y, i)) {
                    memory->game->players[i].position.x = x;
                    memory->game->players[i].position.y = y;
                    printf("Player %d : %d - %d  == %d\n", i, memory->game->players[i].position.x, memory->game->players[i].position.y, memory->game->players[i].msg_id);
                    break;
                }
            }

            memory->game->connected[i] = getpid();
            memory->playerIndex = i;
            signal(SIGINT, player_sigint_callback);
            printf("Connected to game..\n");
            return 0;
        }
    }
    printf("Could not join game as player -- Game is full\n");
    return -1;
}

t_player *get_closest_enemy(t_sharedMemory *memory, int playerIndex) {
    t_player *closestPlayer = NULL;
    int closestDist;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (i == playerIndex)
            continue;
        if (memory->game->connected[i] != 0 && memory->game->players[i].team != memory->game->players[playerIndex].team) {
            // If enemy
            int tmp = get_players_distance(memory, memory->game->players[i], memory->game->players[playerIndex]);
            if (!tmp)
                return NULL;
            if (closestPlayer == NULL || closestDist > tmp) {
                closestDist = tmp;
                closestPlayer = &memory->game->players[i];
            }
        }
    }
    return closestPlayer;
}

t_player *player_on_tile(t_gameData *game, int x, int y, int nCheck) {
    for (int i = 0; (i < MAX_PLAYERS) && i < nCheck; i++) {
        if (game->connected[i] != 0) {
            // printf("Player positions : %d - %d\n", game->players[i].position.x, game->players[i].position.y);
            if (game->players[i].position.x == x && game->players[i].position.y == y)
                return &game->players[i];
        }
    }
    return NULL;
}

char is_player_dead(t_sharedMemory *memory, t_player *player) {
    int count = 0;

    for (int x = -1 ; x <= 1; x++) {
        if (x + player->position.x <= -1 || x + player->position.x >= MAP_WIDTH)
            continue;
        for (int y = -1 ; y <= 1; y++) {
            if (y + player->position.y <= -1 || y + player->position.y >= MAP_HEIGHT)
                continue;
            else if (x == 0 && y == 0)
                continue;
            
            t_player *tmp = player_on_tile(memory->game, x + player->position.x, y + player->position.y, MAX_PLAYERS);
            if (tmp && tmp->team != player->team) {
                count++;
                if (count == 2)
                    return 1;
            }
        }
    }
    return 0;
}



void move_player_to(t_gameData *game, t_player *player, t_coord coord) {
    char isDifferent = (player->position.x != coord.x) || (player->position.y != coord.y);
    player->position.x = coord.x;
    player->position.y = coord.y;
    
    if (isDifferent) {
        printf("\tMoving to %d - %d\n", coord.x, coord.y);
        // Send message to display
        if (game->display_pid != 0) {
            printf("Sending map to display %d\n", game->display_msg_id);
            t_message msg;
            // msg.type = 1;
            msg.data = *game;
            int ret = msgsnd(game->display_msg_id, &msg, sizeof(msg.data), 0);
            if (ret == -1)
                perror("msgsnd (2)");
        }
    }
}

void move_towards_enemy(t_sharedMemory *memory, t_player *player, t_player *enemy) {
    t_coord closest_position = get_closest_position(memory, *player, *enemy);
    t_coord player_position = player->position;
    printf("Player position : %d - %d \t Enemy position : %d - %d \t Closest position : %d - %d\n", player->position.x, player->position.y, enemy->position.x, enemy->position.y, closest_position.x, closest_position.y);

    if (closest_position.x > player->position.x) {
        player_position.x++;
    }
    else if (closest_position.x < player->position.x) {
        player_position.x--;
    }
    else if (closest_position.y > player->position.y) {
        player_position.y++;
    }
    else if (closest_position.y < player->position.y) {
        player_position.y--;
    }

    if (!player_on_tile(memory->game, player_position.x, player_position.y, MAX_PLAYERS)) {
        move_player_to(memory->game, player, player_position);
    }

}

void player_loop(t_sharedMemory *memory) {
    // Game starts
    memory->die = 0;
    t_player *player = &memory->game->players[memory->playerIndex];
    while (1) {
        sleep(1);
        sem_wait(memory->game->semaphore);

        // Player reads instructions and keeps last one


        // Player checks distance with every enemies and send instructions if needed


        // Player checks if he's dead
        // printf("Player %d checking if he's dead\n", memory->playerIndex);
        if (is_player_dead(memory, player) || memory->die) {
            client_dies(memory);
        }

        // Player plays
        // printf("Player %d calculating closest enemy..\n", memory->playerIndex);
        t_player *enemy = get_closest_enemy(memory, memory->playerIndex);
        if (enemy) {
            printf("Player %d playing.. : Closest enemy = %d\n\n", memory->playerIndex, enemy ? enemy->playerIndex : -1);
            move_towards_enemy(memory, &memory->game->players[memory->playerIndex], enemy);
        } else {
            printf("Player %d not playing..\n", memory->playerIndex);
        }
        
        char count = 0;
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (memory->game->connected[i] != 0 && memory->game->players[i].team != player->team) {
                count++;
                break;
            }
        }
        if (!count)
            break;
        sem_post(memory->game->semaphore);
    }
    printf("Congratulations, you won the game !\n");
    client_dies(memory);
    sem_post(memory->game->semaphore);
}