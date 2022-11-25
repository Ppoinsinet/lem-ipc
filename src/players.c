#include <lem.h>

extern t_sharedMemory memory;

t_player *get_closest_enemy(t_sharedMemory *memory, int playerIndex) {
    t_player *closestPlayer = NULL;
    int closestDist;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (i == playerIndex)
            continue;
        if (memory->game->connected[i] != 0 && memory->game->players[i].team != memory->game->players[playerIndex].team) {
            // If enemy
            int tmp = get_players_distance(memory, memory->game->players[i], memory->game->players[playerIndex]);
            if (closestPlayer == NULL || closestDist > tmp) {
                closestDist = tmp;
                closestPlayer = &memory->game->players[i];
            }
        }
    }
    return closestPlayer;
}

void get_players(t_sharedMemory *memory, int msg_ids[MAX_PLAYERS]) {
    printf("Getting players..\n");
    for (int i = 0; i < MAX_PLAYERS; i++) {
        memory->game->players[i].msg_id = msg_ids[i];
        memory->game->players[i].team = i < (MAX_PLAYERS / 2);
        memory->game->players[i].playerIndex = i;
        memory->game->players[i].rank = i % (MAX_PLAYERS / 2);
    }

    for (int j = 0; j < MAX_PLAYERS;) {
        int x = getRandomPosition(MAP_WIDTH);
        int y = getRandomPosition(MAP_HEIGHT);
        if (!player_on_tile(memory, x, y, j)) {
            memory->game->players[j].position.x = x;
            memory->game->players[j].position.y = y;
            printf("Player %d : %d - %d  == %d\n", j, memory->game->players[j].position.x, memory->game->players[j].position.y, memory->game->players[j].msg_id);
            j++;
        }

    }
}

t_player *player_on_tile(t_sharedMemory *memory, int x, int y, int nCheck) {
    for (int i = 0; (i < MAX_PLAYERS) && i < nCheck; i++) {
        if (memory->game->connected[i] != 0 && memory->game->players[i].position.x == x && memory->game->players[i].position.y == y)
            return &memory->game->players[i];
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
            
            t_player *tmp = player_on_tile(memory, x + player->position.x, y + player->position.y, MAX_PLAYERS);
            if (tmp && tmp->team != player->team) {
                count++;
                if (count == 2)
                    return 1;
            }
        }
    }
    return 0;
}

void player_dies(t_sharedMemory *memory, int playerIndex) {
    printf("Player %d dies..\n", playerIndex);
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (i == playerIndex)
            continue;
        if (memory->game->connected[i] != 0) {
            // Another player is connected
            printf("Player %d is not last : %d == %d\n", playerIndex, i, memory->game->connected[i]);
            memory->game->connected[playerIndex] = 0;
            msgsnd(memory->game->display_msg_id, &memory->game, sizeof(t_gameData), 0);
            sem_post(memory->game->semaphore);
            exit(1);
        }
    }

    // Is last player connected -> Has to free IPCs
    printf("Player %d is last\n", playerIndex);
    msgsnd(memory->game->display_msg_id, &memory->game, sizeof(t_gameData), 0);
    clear_ipc(memory);
    sem_post(memory->game->semaphore);
    sem_unlink(SEMAPHORE_NAME);
    exit(0);
}

void move_player_to(t_sharedMemory *memory, t_player *player, t_coord coord) {
    char isDifferent = player->position.x != coord.x || player->position.y != coord.y;
    player->position.x = coord.x;
    player->position.y = coord.y;
    
    if (isDifferent) {
        // Send message to display
        msgsnd(memory->game->display_msg_id, memory->game, sizeof(t_gameData), 0);
    }
}

void move_towards_enemy(t_sharedMemory *memory, t_player *player, t_player *enemy) {
    t_coord closest_position = get_closest_position(memory, *player, *enemy);
    t_coord player_position = player->position;
    printf("Player position : %d - %d \t Closest position : %d - %d\n", player->position.x, player->position.y, closest_position.x, closest_position.y);

    if (closest_position.x > player->position.x) {
        player_position.x++;
    }
    else if (closest_position.x < player->position.x) {
        player_position.x--;
    }

    if (player_on_tile(memory, player->position.x, player->position.y, MAX_PLAYERS)) {
        player_position = player->position;
    }

    if (closest_position.y > player->position.y) {
        player_position.y++;
    }
    else if (closest_position.y < player->position.y) {
        player_position.y--;
    }

    if (player_on_tile(memory, player_position.x, player_position.y, MAX_PLAYERS)) {
        player_position = player->position;
    }

    move_player_to(memory, player, player_position);
}

void player_loop(t_sharedMemory *memory, int playerIndex) {
    // Game starts
    (void)memory;
    t_player *player = &memory->game->players[playerIndex];
    while (1) {
        sem_wait(memory->game->semaphore);

        sleep(1);
        // Player reads instructions and keeps last one


        // Player checks distance with every enemies and send instructions if needed

        // Player checks if he's dead
        if (is_player_dead(memory, player)) {
            player_dies(memory, playerIndex);
        }

        // Player plays
        t_player *enemy = get_closest_enemy(memory, playerIndex);
        printf("Player %d playing.. : Closest enemy = %d\n", playerIndex, enemy->playerIndex);
        move_towards_enemy(memory, &memory->game->players[playerIndex], enemy);



        sem_post(memory->game->semaphore);
    }
}