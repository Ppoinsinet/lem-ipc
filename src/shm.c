#include <lem.h>

key_t getKey(int rd) {
    key_t key = ftok(FILENAME, 0);
    printf("key %d\n", key);
    if (key == IPC_RESULT_ERROR) {
        perror("ftok");
        exit(-4);
    }
    if (rd > -1)
        close(rd);
    return key;
}

t_sharedMemory openSharedMemory(e_command command) {
    int isCreator = 0;
    int rd = -1;
    if (access(FILENAME, F_OK)) {
        // file doesn't exist
        printf("Trying to create game..\n");
        isCreator = 1;

        if ((rd = open(FILENAME, O_CREAT)) == -1) {
            perror("open (1)");
            exit(2);
        }
    }

    int shm_id;
    t_sharedMemory data;
    memset(&data, 0, sizeof(data));
    data.command = command;
    shm_id = shmget(getKey(rd), sizeof(t_gameData), 0600 | IPC_CREAT);
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

    if (isCreator)
        memset(data.game, 0, sizeof(t_gameData));

    printf("attached shared memory %p\n", data.game);
    data.game->shm_id = shm_id;
    printf("Got shared memory ID %d\n", data.game->shm_id);
    return data;
}

void shared_memory_init(t_sharedMemory *memory) {
    if (memory->isCreator) {
        sem_unlink(SEMAPHORE_NAME);
        memory->game->semaphore = sem_open(SEMAPHORE_NAME, O_RDWR | O_CREAT, 0644, 1);
        if (memory->game->semaphore == SEM_FAILED) {
            perror("sem_open (1)");
            exit(-7);
        }
        if (sem_init(memory->game->semaphore, 1, 1) == -1) {
            perror("sem_init");
            exit(-8);
        }
        
        memory->game->display_pid = 0;
        memory->game->display_msg_id = 0;
    } else {
        // Not creator
        memory->game->semaphore = sem_open(SEMAPHORE_NAME, 0);

    }
}