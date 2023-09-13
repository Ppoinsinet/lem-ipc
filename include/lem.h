#ifndef LEM_H
# define LEM_H

# define MAP_WIDTH 10
# define MAP_HEIGHT 10

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <string.h>

#include <sys/mman.h>
#include <fcntl.h>
#include <sys/time.h>

#include <errno.h>

# define IPC_RESULT_ERROR -1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>


# define MIN_PLAYERS 2
# define MAX_PLAYERS (MAP_WIDTH * MAP_HEIGHT)
# define FILENAME "/tmp/sharedmemory.ppoinsin"
# define SEMAPHORE_NAME "sharememorysemaphore.ppoinsin"

typedef enum {
    CMD_NONE,
    CMD_TEAM_1,
    CMD_TEAM_2
} e_command;

typedef enum {
    PLAYER_TURN,
    DISPLAY_TURN
} e_turn;

#define ANSI_GREEN "\033[0;32m"
#define ANSI_RED "\033[0;31m"
#define ANSI_RESET "\033[0;0m"

typedef struct {
    int x;
    int y;
} t_coord;

typedef struct {
    t_coord position;
    int msg_id;
    int shm_id;
    pid_t pid;
    int isDead;
    int team;
    int playerIndex;
    int targetIndex;
    e_command command;
    sem_t *semaphore;
} t_playerStatus;

typedef int t_mapTile;

typedef struct {
    t_coord pos;
    int distance;
} t_pathFindingResult;

typedef struct {
    unsigned char n_players;
    char start;
    char finished;
    e_turn turn;
    long int nextTurnTimestamp;

    t_mapTile map[MAP_WIDTH * MAP_HEIGHT];
    t_playerStatus players[MAX_PLAYERS];

} t_gameData;

typedef struct {
    int enemyIndex;
    int distance;
    int providerIndex;
} t_orderMessagePayload;

typedef struct {
    int distance;
    t_playerStatus *enemy;
} t_closestEnemy;

// SHM
t_gameData *openGame();
void shared_memory_init();

// Display
void print_map(t_mapTile *map);

// Utils
int getRandomPosition(int max);
t_pathFindingResult getDistance(t_coord a, t_coord b, t_pathFindingResult limit);

// Players
t_playerStatus *getTile(int x, int y);
t_closestEnemy getClosestEnemy();
void playerLoop();
int player_init();

// Errors
void incorrect_usage_error(void);
void onError(const char *str, const char freeSemaphore);

// Parsing
void parseCommand(int ac, char **av);

extern t_playerStatus status;
extern t_gameData *game;

// Init
void initSemaphore();
void initCreator();
void initPlayer();

void waitAllPlayers();

t_coord pathFinding(t_playerStatus from, t_playerStatus to);
t_pathFindingResult pathFindingResult(int distance, t_coord coord);

t_coord get_coord(int x, int y);

// Clear
char closeMessageQueue(int id);
char closeSharedMemory(int id);

// Orders
void emitOrder(t_closestEnemy *target);
t_closestEnemy readOrders();

#endif