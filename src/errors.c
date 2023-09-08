#include <lem.h>

void incorrect_usage_error(void) {
    printf("Incorrect usage\n");
    printf("Usage: ./lemipc <arg>\n\n");
    printf("arg:\t1 -- Join team 1\n");
    printf("    \t2 -- Join team 2\n");
    printf("    \tdisplay -- Join as active display\n");
    exit(1);
}

void onError(const char *str, const char freeSemaphore) {
    if (freeSemaphore)
        sem_post(status.semaphore);
    perror(str);
    exit(1);
}