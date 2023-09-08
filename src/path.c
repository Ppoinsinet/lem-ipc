#include <lem.h>

static char isNextToTarget(t_coord from, t_coord to) {
    int x_distance = abs(from.x - to.x);
    int y_distance = abs(from.y - to.y);
    return (x_distance == 1 && y_distance == 0) || (x_distance == 0 && y_distance == 1);
}

static char wasPositionAlreadyUsed(t_coord arr[MAP_WIDTH * MAP_HEIGHT], int length, t_coord pos) {
    for (int i = 0; i < length; i++) {
        if (arr[i].x == pos.x && arr[i].y == pos.y) {
            return 1;
        }
    }
    return 0;
}

static t_pathFindingResult getLimit(t_pathFindingResult arr[4]) {
    t_pathFindingResult r = pathFindingResult(-1, get_coord(0,0));

    for (int i = 0; i < 4; i++)
        if (arr[i].distance > -1 && (r.distance == -1 || arr[i].distance < r.distance))
            r = arr[i];
    return r;
}

/**
 * @brief Returns a vector to perform a mouvement
 * 
 * @param from 
 * @param to 
 * @return t_coord 
 */
t_coord pathFinding(t_playerStatus from, t_playerStatus to) {
    t_pathFindingResult results[4] = { 
        pathFindingResult(-1, get_coord(0,0)),
        pathFindingResult(-1, get_coord(0,0)),
        pathFindingResult(-1, get_coord(0,0)),
        pathFindingResult(-1, get_coord(0,0))
    };

    t_pathFindingResult min = getDistance(from.position, to.position, getLimit(results));
    if (min.distance > -1) {
        return min.pos;
    }
    return get_coord(0, 0);
}

void recursive(t_pathFindingResult *pathToTake, t_coord from, t_coord to, int xOffset, int yOffset) {

    t_pathFindingResult tmp = getDistance(get_coord(from.x + xOffset, from.y + yOffset), to, *pathToTake);
    if (tmp.distance > -1 && (pathToTake->distance == -1 || tmp.distance < pathToTake->distance)) {
        pathToTake->distance = tmp.distance;
        pathToTake->pos = get_coord(xOffset, yOffset);
    }
}

t_pathFindingResult getDistance(t_coord from, t_coord to, t_pathFindingResult limit) {
    static t_coord lastPositions[MAP_WIDTH * MAP_HEIGHT];
    static int length = 0;

    if (from.x <= -1 || from.x >= MAP_WIDTH || from.y <= -1 || from.y >= MAP_HEIGHT) {
        return pathFindingResult(-1, get_coord(0, 0));
    }
    
    if (limit.distance > -1 && length + 1 >= limit.distance)
        return pathFindingResult(-1, get_coord(0, 0));
    
    // Check if another player is on tile
    if (length && getTile(from.x, from.y)) {
        return pathFindingResult(-1, get_coord(0, 0));
    }

    if (isNextToTarget(from, to)) {
        return pathFindingResult(length, get_coord(0,0));
    }

    // Checking if position was already used
    if (wasPositionAlreadyUsed(lastPositions, length, from))
        return pathFindingResult(-1, get_coord(0, 0));

    lastPositions[length++] = from;
    t_pathFindingResult pathToTake = limit;

    if (to.x < from.x) {
        // Start from left to right
        recursive(&pathToTake, from, to, -1, 0);
        recursive(&pathToTake, from, to, 1, 0);

    } else {

        // Start from right to left
        recursive(&pathToTake, from, to, 1, 0);
        recursive(&pathToTake, from, to, -1, 0);
    }

    if (to.y < from.y) {

        // Start from bottom to top
        recursive(&pathToTake, from, to, 0, -1);
        recursive(&pathToTake, from, to, 0, 1);
        
    } else {

        // Start from top to bottom
        recursive(&pathToTake, from, to, 0, 1);
        recursive(&pathToTake, from, to, 0, -1);
    }
   
    length--;
    
    return pathToTake;
}