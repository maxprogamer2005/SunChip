CC = gcc
CFLAGS=-std=c89 -Wall -Wextra -Werror

all:
	${CC} src/main.c -o sunchip -lSDL3 ${CFLAGS}
