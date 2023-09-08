SRC = src/main.c src/shm.c src/display.c src/utils.c src/players.c src/errors.c src/parsing.c src/init.c src/path.c

OBJS = $(SRC:.c=.o)

NAME = lemipc

CC = clang

FLAGS = -Wall -Wextra -Werror -g

INCLUDE = include/

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(FLAGS) -o $(NAME) $(OBJS)

.c.o:
	$(CC) $(FLAGS) $< -c -o $(<:.c=.o) -I$(INCLUDE)

clean:
	rm -rf $(OBJS)

fclean: clean
	rm -rf $(NAME)

re: fclean $(NAME)

.PHONY: all clean fclean re # Not representing files