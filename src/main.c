 #include <lem.h>

t_sharedMemory memory;

void set_map_value(pid_t *map, int x, int y, int val) {
    map[y * MAP_WIDTH + x] = val;
}

int main() {
    srand(time(NULL));

    memory = openSharedMemory();

    int pid = getpid();
    if (memory.isCreator)
        printf("Parent PID : %d\n", pid);
    else
        printf("Child PID : %d\n", pid);

    for (int i = 0 ; i < MAP_HEIGHT * MAP_WIDTH; i++) {
        memory.game->map[i] = 0;
    }
    printf("Map initialized\n");

    if (memory.isCreator) {
        printf("Creator\n");
        
        shared_memory_init(&memory);
        creator_entry(&memory);

    } else {
        printf("Child\n");
        child_entry(&memory);
    }

    // t_player players[4];

    // get_players(players, msg_ids);

    // int isParent = fork();

    // if (!isParent) {
    //     childLoop(players[i]);
    //     return 0;
    // }
    // else {
    //     players[i].pid = isParent;
    //     set_map_value(map, players[i].x, players[i].y, players[i].pid);
    // }
    // print_map(memory.map, players);

    return 0;

}