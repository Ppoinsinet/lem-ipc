#include <lem.h>

int is_last_client(t_sharedMemory *memory) {
    pid_t pid = getpid();
    if (memory->game->display_pid != 0 && memory->game->display_pid != pid)
        return 0;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (memory->game->connected[i] != 0 && memory->game->connected[i] != pid) {
            // Another player is connected
            printf("Client %d is not last : %d == %d\n", memory->playerIndex, i, memory->game->connected[i]);
            return 0;
        }
    }
    // Client is last connected
    printf("Client %d is last\n", memory->playerIndex);
    return 1;
}

int is_player(t_sharedMemory *memory) {
    pid_t pid = getpid();

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (memory->game->players[i].pid == pid)
            return 1;
    }
    return 0;
}

void client_dies(t_sharedMemory *memory) {
    printf("Client %d dies..\n", memory->playerIndex);

    
    // Deletes own message queue
    if (memory->command == CMD_DISPLAY) {
        printf("Unsetting display message queue %d\n", memory->game->display_msg_id);
        msgctl(memory->game->display_msg_id, IPC_RMID, NULL);
    }
    else {
        printf("Unsetting player message queue %d\n", memory->game->players[memory->playerIndex].msg_id);
        msgctl(memory->game->players[memory->playerIndex].msg_id, IPC_RMID, NULL);
    }

    if (!is_last_client(memory)) {
        // Is not last client connected -> No need to clear SharedMemory
        memory->game->connected[memory->playerIndex] = 0;
        if (memory->game->display_pid != 0)
            // if (msgsnd(memory->game->display_msg_id, memory->game, sizeof(t_gameData), 0) == -1)
            //     perror("msgsnd (yo)");
        sem_post(memory->game->semaphore);
    } else {
        // Is last client connected -> Has to clear SharedMemory
        clear_ipc(memory);
        sem_post(memory->game->semaphore);
        sem_unlink(SEMAPHORE_NAME);
    }
    exit(0);
}