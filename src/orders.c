#include <lem.h>

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
            
            if (msg.enemyIndex >= 0 && msg.providerIndex < providerIndex) {
                printf("ORDER - SETTING TARGET TO : %d\n", msg.enemyIndex);
                target.enemy = &game->players[msg.enemyIndex];
                target.distance = msg.distance;
                providerIndex = msg.providerIndex;
            }
        }
    }
    return target;
}