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
    CMD_TEAM_2,
    CMD_DISPLAY
} e_command;

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
    pid_t pid;
    int team;
    int playerIndex;
    char connected;
} t_player;

typedef struct {
    int playerIndex;
    char team;
} t_mapTile;

typedef struct {
    sem_t *semaphore;
    int shm_id;

    t_mapTile map[MAP_WIDTH * MAP_HEIGHT];

    t_player players[MAX_PLAYERS];
    pid_t connected[MAX_PLAYERS];
    unsigned char n_players;

    pid_t display_pid;
    int display_msg_id;
    char start;
} t_gameData;

typedef struct {
    int isCreator;
    e_command command;
    t_gameData *game;

    int playerIndex;
    int shm_id;
    char die;
} t_sharedMemory;

typedef enum {
    MSG_NONE,
    MSG_PLAY,
    MSG_CONNECT,
    MSG_DIE
} e_displayMessageType;

typedef struct {
    e_displayMessageType type;
    t_mapTile map[MAP_WIDTH * MAP_HEIGHT];
    int playerIndex; // Index of player playing
} t_displayMessagePayload;

typedef struct {
    int enemyIndex;
    int distance;
    int providerIndex;
} t_orderMessagePayload;

union messagePayload {
    t_displayMessagePayload data;
    t_orderMessagePayload order;
};

typedef struct {
    long type;
    union messagePayload payload;
    // t_displayMessagePayload data;
} t_message;

typedef struct {
    int distance;
    t_player *enemy;
} t_closestEnemy;

// t_targetMessage {

// }


// Creator
void creator_entry(t_sharedMemory *memory);

// Child
void child_entry(t_sharedMemory *memory);

// IPCs
void clear_ipc(t_sharedMemory *memory);

// SHM
t_sharedMemory openSharedMemory(e_command command);
void shared_memory_init(t_sharedMemory *memory);

// Display
void print_map(t_mapTile *map);
void display_loop(t_sharedMemory *memory);
int display_init(t_sharedMemory *memory);

// Utils
int getRandomPosition(int max);
int get_players_distance(t_sharedMemory *memory, t_player a, t_player b);
int get_distance_to(int x1, int y1, int x2, int y2);
t_coord get_closest_position(t_sharedMemory *memory, t_player from, t_player to);
void player_sigint_callback(int arg);
void display_sigint_callback(int arg);
int is_there_obstacle_between(t_sharedMemory *memory, t_coord from, t_coord to);

// Players
t_player *player_on_tile(t_gameData *game, int x, int y);
t_closestEnemy get_closest_enemy(t_sharedMemory *memory, int playerIndex);
void player_loop(t_sharedMemory *memory);
void client_dies(t_sharedMemory *memory);
int player_init(t_sharedMemory *memory);

// Errors
void incorrect_usage_error(void);

// Clients
int is_last_client(t_sharedMemory *memory);
int is_player(t_sharedMemory *memory);

int send_display_message(int msg_id, int player_index, int type, t_mapTile *map);



#endif