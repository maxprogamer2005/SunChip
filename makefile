CC = clang
CPPFLAGS=-std=c++23 -Wall -Wextra -Werror

all:
	clang main.cpp -o sunchip -lSDL3 ${CPPFLAGS}
