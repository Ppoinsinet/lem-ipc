#include <lem.h>

int send_display_message(int msg_id, int player_index, int type, t_mapTile *map) {
    if (msg_id <= 0)
        return 1;
    t_displayMessage msg;
    msg.type = 1;
    memcpy(msg.data.map, map, sizeof(msg.data.map));
    msg.data.type = type;
    msg.data.playerIndex = player_index;
    if (msgsnd(msg_id, &msg, sizeof(msg.data), 0) == -1) {
        perror("msgsnd");
        return 1;
    }
    return 0;
}

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
        print_map(memory->game->map);
        return 1;
    }
}

void print_map(t_mapTile *map) {
    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j= 0; j < MAP_WIDTH; j++) {
            if (map[j + i * MAP_WIDTH].playerIndex != -1) {
                if (map[j + i * MAP_WIDTH].team == 1)
                    printf("%s%d%s ", ANSI_RED, map[j + i * MAP_WIDTH].playerIndex,ANSI_RESET);
                else
                    printf("%s%d%s ", ANSI_GREEN, map[j + i * MAP_WIDTH].playerIndex, ANSI_RESET);
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
    print_map(memory->game->map);
    sem_post(memory->game->semaphore);

    printf("Entering display loop.. on %d\n", display_msgid);

    struct msqid_ds buf;
    msgctl(display_msgid, IPC_STAT, &buf);
    printf("Message size : %lu\n", buf.msg_qbytes);
    
    t_displayMessage msg;
    int ret;
    while ((ret = msgrcv(display_msgid, &msg, sizeof(msg.data), 0, 0)) != -1) {
        if (msg.data.type == MSG_CONNECT)
            printf("Player %d connected..\n", msg.data.playerIndex);
        else if (msg.data.type == MSG_DIE)
            printf("Player %d died..\n", msg.data.playerIndex);
        else if (msg.data.type == MSG_PLAY)
            printf("Player %d playing..\n", msg.data.playerIndex);
        // use message queues
        print_map(msg.data.map);
    }
    perror("msgrcv");
    client_dies(memory);
}