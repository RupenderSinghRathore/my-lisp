CC = gcc
CCFLAGS = -g -Wall -Wextra -Wpedantic -fsanitize=address,undefined -fno-omit-frame-pointer -Ivendor/mpc

SRC := src/*.c vendor/mpc/*.c

run:
	$(CC) $(CCFLAGS) $(SRC) -ledit -o run && ./run; rm ./run
