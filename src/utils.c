#include <lem.h>

#include <math.h>

int getRandomPosition(int max) {
    return rand() % max;
}

int get_distance_to(int x1, int y1, int x2, int y2) {
    return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
}

t_coord get_coord(int x, int y) {
    t_coord tmp;
    tmp.x = x;
    tmp.y = y;
    return tmp;
}

// TODO : use libft
int get_players_distance(t_sharedMemory *memory, t_player from, t_player to) {

    int dist = MAP_WIDTH + MAP_HEIGHT;

    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            if (x == 0 && y == 0)
                continue;
            if (!player_on_tile(memory, to.position.x + x, to.position.y + y, MAX_PLAYERS)) {
                int tmp = get_distance_to(from.position.x, from.position.y, to.position.x + x, to.position.y + y < dist);
                if (tmp < dist)
                    dist = tmp;
            }
        }
    }
    return dist;
}

// TODO : use libft
t_coord get_closest_position(t_sharedMemory *memory, t_player from, t_player to) {

    int dist = MAP_WIDTH + MAP_HEIGHT;
    t_coord coord = {-1, -1};

    for (int x = -1; x <= 1; x++) {

        if (to.position.x + x <= -1 || to.position.x >= MAP_WIDTH)
            continue;
        for (int y = -1; y <= 1; y++) {
            if (to.position.y + y <= -1 || to.position.y >= MAP_HEIGHT)
                continue;
            if (x == 0 && y == 0)
                continue;
            if (!player_on_tile(memory, to.position.x + x, to.position.y + y, MAX_PLAYERS)) {
                int tmp = get_distance_to(from.position.x, from.position.y, to.position.x + x, to.position.y + y);
                if (tmp < dist) {
                    dist = tmp;
                    coord = get_coord(to.position.x + x, to.position.y + y);
                }
            }
        }
    }
    return coord;
}

