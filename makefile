CC = clang
CFLAGS=-std=c90 -Wall -Wextra -Werror

all:
	clang main.c -o sunchip -lSDL3 ${CFLAGS}
