 #include <lem.h>

t_sharedMemory memory;

int main(int ac, char **av) {
    printf("Starting program..\n");
    e_command command = CMD_NONE;
    if (ac != 2)
        incorrect_usage_error();
    else if (!strcmp(av[1], "1"))
        command = CMD_TEAM_1;
    else if (!strcmp(av[1], "2"))
        command = CMD_TEAM_2;
    else if (!strcmp(av[1], "display"))
        command = CMD_DISPLAY;
    else
        incorrect_usage_error();

    srand(time(NULL));
    memory = openSharedMemory(command);

    int pid = getpid();
    shared_memory_init(&memory);
    
    if (memory.isCreator) {
        printf("Parent PID : %d\n", pid);
        creator_entry(&memory);

    }
    else {
        printf("Child PID : %d\n", pid);
        child_entry(&memory);
    }

    return 0;

}