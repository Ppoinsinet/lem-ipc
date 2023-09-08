#include <lem.h>

void print_map(t_mapTile *map) {
    static int turn = 1;

    printf("\nTurn %d\n", turn++);
    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j= 0; j < MAP_WIDTH; j++) {

            int index = map[j + i * MAP_WIDTH];
            if (index >= 0)
                printf("%s%d%s ", game->players[index].team == 1 ? ANSI_RED : ANSI_GREEN, game->players[index].team, ANSI_RESET);
            else
                printf("- ");
        }
        printf("\n");
    }
    printf("\n");
}