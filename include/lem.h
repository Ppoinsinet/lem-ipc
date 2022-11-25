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

#include <sys/mman.h>
#include <fcntl.h>

# define IPC_RESULT_ERROR -1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>


# define MAX_PLAYERS 4
# define FILENAME "/tmp/sharedmemory.ppoinsin"
# define SEMAPHORE_NAME "sharememorysemaphore.ppoinsin"

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
    int rank;
    char connected;
} t_player;

typedef struct {
    sem_t *semaphore;
    int shm_id;
    int map[MAP_WIDTH * MAP_HEIGHT];
    t_player players[MAX_PLAYERS];
    char dead[MAX_PLAYERS];
    pid_t connected[MAX_PLAYERS];
    unsigned char n_players;

    pid_t display_pid;
    int display_msg_id;
    char start;
} t_gameData;

typedef struct {
    t_gameData *game;

    int isCreator;
    int msg_ids[MAX_PLAYERS];
    int shm_id;
} t_sharedMemory;


// Creator
void creator_entry(t_sharedMemory *memory);

// Child
void child_entry(t_sharedMemory *memory);

// IPCs
void clear_ipc(t_sharedMemory *memory);

// SHM
t_sharedMemory openSharedMemory(void);
void shared_memory_init(t_sharedMemory *memory);

// Display
void print_map(t_sharedMemory *memory);
void display_loop(t_sharedMemory *memory);

// Utils
int getRandomPosition(int max);
int get_players_distance(t_sharedMemory *memory, t_player a, t_player b);
int get_distance_to(int x1, int y1, int x2, int y2);
t_coord get_closest_position(t_sharedMemory *memory, t_player from, t_player to);
void sigint_callback(int arg);

// Players
void get_players(t_sharedMemory *memory, int msg_ids[MAX_PLAYERS]);
t_player *player_on_tile(t_sharedMemory *memory, int x, int y, int nCheck);
t_player *get_closest_enemy(t_sharedMemory *memory, int playerIndex);
void player_loop(t_sharedMemory *memory, int playerIndex);
void player_dies(t_sharedMemory *memory, int playerIndex);



#endif