CC = gcc
DEVLIB = lSDL3

ifeq ($(DEVLIB),lSDL3)
	LIBMAIN = sdl3main.c
endif

CFLAGS=-std=c89 -Wall -Wextra -Werror

all:
	${CC} src/${LIBMAIN} -o sunchip -${DEVLIB} ${CFLAGS}
