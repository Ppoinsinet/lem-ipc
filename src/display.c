#include <lem.h>

int display_init(t_sharedMemory *memory) {
    if (memory->game->display_pid == 0) {
        printf("Joining as active display..\n");
        memory->game->display_msg_id = msgget(IPC_PRIVATE, 0700 | IPC_CREAT);
        if (memory->game->display_msg_id == -1) {
            perror("msgget (display)");
            return 1;
        }
        printf("Display setup %d message queue\n", memory->game->display_msg_id);
        memory->playerIndex = -1;
        memory->game->display_pid = getpid();
        signal(SIGINT, display_sigint_callback);
        return 0;
    } else {
        printf("Active display is already running.. Printing map..\n");
        print_map(memory->game);
        return 1;
    }
}

void print_map(t_gameData *game) {
    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j= 0; j < MAP_WIDTH; j++) {
            const t_player *player = player_on_tile(game, j, i, MAX_PLAYERS);
            if (player) {
                if (player->team == 1)
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
    print_map(memory->game);
    sem_post(memory->game->semaphore);

    printf("Entering display loop.. on %d\n", display_msgid);

    struct msqid_ds buf;
    msgctl(display_msgid, IPC_STAT, &buf);
    printf("Message size : %lu\n", buf.msg_qbytes);
    
    t_message msg;
    int ret;
    while ((ret = msgrcv(display_msgid, &msg.data, sizeof(msg.data), 0, 0)) != -1) {
        printf("Received %d\n", ret);
        // use message queues
        print_map(&msg.data);
    }
    perror("msgrcv");
    client_dies(memory);
}