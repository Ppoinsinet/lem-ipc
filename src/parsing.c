#include <lem.h>

void parseCommand(int ac, char **av) {
    if (ac != 2)
        incorrect_usage_error();
    else if (!strcmp(av[1], "1")) {
        status.command = CMD_TEAM_1;
        status.team = 1;
    }
    else if (!strcmp(av[1], "2")) {
        status.command = CMD_TEAM_2;
        status.team = 2;
    }
    else
        incorrect_usage_error();
}