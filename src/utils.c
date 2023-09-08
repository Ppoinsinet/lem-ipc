#include <lem.h>

#include <math.h>

int getRandomPosition(int max) {
    return rand() % max;
}

t_coord get_coord(int x, int y) {
    t_coord tmp;
    tmp.x = x;
    tmp.y = y;
    return tmp;
}

t_pathFindingResult pathFindingResult(int distance, t_coord coord) {
    t_pathFindingResult r;
    r.pos = coord;
    r.distance = distance;
    return r;
}

