#include <lem.h>

#include <math.h>

int getRandomPosition(int max) {
    return rand() % max;
}

int get_distance_to(int x1, int y1, int x2, int y2) {
    printf("Distance (%d, %d) et (%d, %d) == %d\n", x1, y1, x2, y2, ((x1 > x2 ? x1 - x2 : x2 - x1) + (y1 > y2 ? y1 - y2 : y2 - y1)));
    return ((x1 > x2 ? x1 - x2 : x2 - x1) + (y1 > y2 ? y1 - y2 : y2 - y1));
}

t_coord get_coord(int x, int y) {
    t_coord tmp;
    tmp.x = x;
    tmp.y = y;
    return tmp;
}

int get_players_distance(t_sharedMemory *memory, t_player from, t_player to) {
    printf("Looking for distance between %d and %d\n", from.playerIndex, to.playerIndex);
    int x_distance = from.position.x > to.position.x ? from.position.x - to.position.x : to.position.x - from.position.x;
    int y_distance = from.position.y > to.position.y ? from.position.y - to.position.y : to.position.y - from.position.y;

    if (x_distance <= 1 && y_distance <= 1) {
        printf("Already to closest enemy\n");
        return 0;
    }

    int dist = MAP_WIDTH * MAP_HEIGHT;

    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            t_player *player = player_on_tile(memory->game, to.position.x + x, to.position.y + y);
            if (!player) {
                int tmp = get_distance_to(from.position.x, from.position.y, to.position.x + x, to.position.y + y);
                printf("Calculating distance to %d = %d\n", to.playerIndex, tmp);
                if (tmp < dist)
                    dist = tmp;
            }
        }
    }
    printf("Closest enemy distance %d\n", dist);
    return dist;
}

// TODO : use libft
t_coord get_closest_position(t_sharedMemory *memory, t_player from, t_player to) {

    int dist = MAP_WIDTH + MAP_HEIGHT;
    t_coord coord = {-1, -1};

    for (int x = -1; x <= 1; x++) {

        if (to.position.x + x <= -1 || to.position.x  + x >= MAP_WIDTH)
            continue;
        for (int y = -1; y <= 1; y++) {
            if (to.position.y + y <= -1 || to.position.y + y >= MAP_HEIGHT)
                continue;
            if (x == 0 && y == 0)
                continue;
            if (!player_on_tile(memory->game, to.position.x + x, to.position.y + y)) {
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

