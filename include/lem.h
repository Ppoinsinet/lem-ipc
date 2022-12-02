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
} t_command;

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
    sem_t *semaphore;
    int shm_id;

    t_player players[MAX_PLAYERS];
    pid_t connected[MAX_PLAYERS];
    unsigned char n_players;

    pid_t display_pid;
    int display_msg_id;
    char start;
} t_gameData;

typedef struct {
    int isCreator;
    t_command command;
    t_gameData *game;

    int playerIndex;
    int shm_id;
    char die;
} t_sharedMemory;

typedef struct {
    // long int type;
    t_gameData data;
} t_message;


// Creator
void creator_entry(t_sharedMemory *memory);

// Child
void child_entry(t_sharedMemory *memory);

// IPCs
void clear_ipc(t_sharedMemory *memory);

// SHM
t_sharedMemory openSharedMemory(t_command command);
void shared_memory_init(t_sharedMemory *memory);

// Display
void print_map(t_gameData *game);
void display_loop(t_sharedMemory *memory);
int display_init(t_sharedMemory *memory);

// Utils
int getRandomPosition(int max);
int get_players_distance(t_sharedMemory *memory, t_player a, t_player b);
int get_distance_to(int x1, int y1, int x2, int y2);
t_coord get_closest_position(t_sharedMemory *memory, t_player from, t_player to);
void player_sigint_callback(int arg);
void display_sigint_callback(int arg);

// Players
t_player *player_on_tile(t_gameData *game, int x, int y, int nCheck);
t_player *get_closest_enemy(t_sharedMemory *memory, int playerIndex);
void player_loop(t_sharedMemory *memory);
void client_dies(t_sharedMemory *memory);
int player_init(t_sharedMemory *memory);

// Errors
void incorrect_usage_error(void);

// Clients
int is_last_client(t_sharedMemory *memory);
int is_player(t_sharedMemory *memory);



#endif