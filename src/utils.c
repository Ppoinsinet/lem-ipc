#include <lem.h>

#include <math.h>

int getRandomPosition(int max) {
    return rand() % max;
}

int get_distance_to(int x1, int y1, int x2, int y2) {
    return ((x1 > x2 ? x1 - x2 : x2 - x1) + (y1 > y2 ? y1 - y2 : y2 - y1));
}

t_coord get_coord(int x, int y) {
    t_coord tmp;
    tmp.x = x;
    tmp.y = y;
    return tmp;
}

int get_players_distance(t_sharedMemory *memory, t_player from, t_player to) {
    int x_distance = from.position.x > to.position.x ? from.position.x - to.position.x : to.position.x - from.position.x;
    int y_distance = from.position.y > to.position.y ? from.position.y - to.position.y : to.position.y - from.position.y;

    if (x_distance <= 1 && y_distance <= 1) {
        return 0;
    }

    int dist = MAP_WIDTH * MAP_HEIGHT;

    for (int x = -1; x <= 1; x++) {
        if ((to.position.x + x <= -1) || (to.position.x + x >= MAP_WIDTH))
            continue;
        for (int y = -1; y <= 1; y++) {
            if ((to.position.y + y <= -1) || (to.position.y + y >= MAP_HEIGHT))
                continue;
            t_player *player = player_on_tile(memory->game, to.position.x + x, to.position.y + y);
            t_coord tmpCoord = {.x = to.position.x + x, to.position.y + y};
            if (!player && !is_there_obstacle_between(memory, from.position, tmpCoord)) {
                int tmp = get_distance_to(from.position.x, from.position.y, to.position.x + x, to.position.y + y);
                if (tmp < dist)
                    dist = tmp;
            }
            else if (player->playerIndex == from.playerIndex)
                return 0;
        }
    }
    return dist;
}

int is_there_obstacle_between(t_sharedMemory *memory, t_coord from, t_coord to) {
    while (from.x != to.x && from.y != to.y) {
        if (from.x < to.x) {
            from.x++;
        }
        else if (from.x > to.x) {
            from.x--;
        }
        else if (from.y < to.y) {
            from.y++;
        }
        else if (from.y > to.y) {
            from.y--;
        }

        if (from.x != to.x && from.y != to.y && player_on_tile(memory->game, from.x, from.y)) {
            return 1;
        }
    }
    return 0;
}

// TODO : use libft
t_coord get_closest_position(t_sharedMemory *memory, t_player from, t_player to) {

    int dist = MAP_WIDTH + MAP_HEIGHT;
    t_coord coord = {-1, -1};

    for (int x = -1; x <= 1; x++) {

        if ((to.position.x + x <= -1) || (to.position.x  + x >= MAP_WIDTH))
            continue;
        for (int y = -1; y <= 1; y++) {
            if ((to.position.y + y <= -1) || (to.position.y + y >= MAP_HEIGHT))
                continue;
            t_player *player = player_on_tile(memory->game, to.position.x + x, to.position.y + y);
            if (!player) {
                int tmp = get_distance_to(from.position.x, from.position.y, to.position.x + x, to.position.y + y);
                t_coord tmpCoord = {.x = to.position.x + x, .y = to.position.y + y};
                if (tmp < dist && !is_there_obstacle_between(memory, from.position, tmpCoord)) {
                    dist = tmp;
                    coord = get_coord(to.position.x + x, to.position.y + y);
                }
            } else if (player->playerIndex == from.playerIndex) {
                return from.position;
            }
        }
    }
    return coord;
}

