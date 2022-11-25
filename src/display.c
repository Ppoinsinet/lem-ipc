#include <lem.h>

void print_map(t_sharedMemory *memory) {
    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j= 0; j < MAP_WIDTH; j++) {
            const t_player *player = player_on_tile(memory, j, i, MAX_PLAYERS);
            if (player) {
                if (player->team)
                    printf("%s%d%s ", ANSI_RED, player->playerIndex,ANSI_RESET);
                else
                    printf("%s%d%s ", ANSI_GREEN, player->playerIndex, ANSI_RESET);
            }
            else
                printf("- ");
        }
        printf("\n");
    }
    printf("\n");
}

void display_loop(t_sharedMemory *memory) {
    sem_wait(memory->game->semaphore);
    int display_msgid = memory->game->display_msg_id;
    sem_post(memory->game->semaphore);
    while (1) {
        // use message queues
        t_sharedMemory mem;

        msgrcv(display_msgid, &mem, sizeof(mem), 0, 0);
        printf("Received message\n");

        print_map(&mem);

        

        sleep(5);
    }
}