#include <lem.h>

t_playerStatus status = {
    .msg_id = -1,
    .shm_id = -1,
    .pid = -1,
    .isDead = 0,
    .team = -1,
    .playerIndex = -1,
    .targetIndex = -1,
    .command = CMD_NONE,
};

t_gameData *game = NULL;

int main(int ac, char **av) {
    printf("Starting program..\n");
    parseCommand(ac, av);

    srand(time(NULL));
    status.pid = getpid();
    game = openGame();

    initSemaphore();
    if (!status.playerIndex) {
        // Is creator of game
        initCreator();
        playerLoop();
        
    } else {
        // Is not creator of game
        initPlayer();
        playerLoop();
    }
    
    return 0;
}