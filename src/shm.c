#include <lem.h>

t_sharedMemory openSharedMemory() {
    int isCreator = 0;
    int rd;

    if (access(FILENAME, F_OK)) {
        // file doesn't exist
        printf("Trying to create game..\n");
        rd = open(FILENAME, O_CREAT, 0666);
        if (rd == -1) {
            perror("open (2)");
            exit(-5);
        }
        isCreator = 1;
    }
    
    key_t key = ftok(FILENAME, 0);
    if (key == IPC_RESULT_ERROR) {
        perror("ftok");
        exit(-4);
    }
    close(rd);

    int shm_id;
    t_sharedMemory data;
    shm_id = shmget(key, sizeof(t_gameData), 0600 | IPC_CREAT);
    if (shm_id == -1) {
        perror("shmget");
        exit(-3);
    }

    data.isCreator = isCreator;
    data.game = shmat(shm_id, NULL, 0);
    if (data.game == NULL) {
        perror("shmat");
        exit(-6);
    }
    data.game->shm_id = shm_id;
    return data;
}

void shared_memory_init(t_sharedMemory *memory) {
    sem_unlink(SEMAPHORE_NAME);

    memory->game->display_pid = 0;
    memory->game->display_msg_id = 0;


    memory->game->semaphore = sem_open(SEMAPHORE_NAME, IPC_CREAT, 0660, 1);
    if (memory->game->semaphore == SEM_FAILED) {
        perror("sem_open");
        exit(-7);
    }

    memory->game->display_msg_id = msgget(IPC_PRIVATE, 0600);
    if (memory->game->display_msg_id == -1) {
        perror("msgget (1)");
        exit(3);
    }


    for (int i = 0; i < (MAX_PLAYERS); i++) {
        memory->msg_ids[i] = msgget(IPC_PRIVATE, 0600);
        if (memory->msg_ids[i] == -1) {
            perror("msgget (2)");
            exit(2);
        }
        // printf("Set message queue : %d\n", memory->msg_ids[i]);
        memory->game->connected[i] = 0;
    }
    memory->game->connected[0] = getpid();
}