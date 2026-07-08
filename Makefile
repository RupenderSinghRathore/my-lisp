CC = gcc

WARNINGS = -Wall -Wextra -Wpedantic
MEMORY_CHECKS = -fsanitize=address,undefined  -fno-omit-frame-pointer 
INCLUDES = -Ivendor/mpc
SRC := src/*.c vendor/mpc/*.c
LIBS = -ledit -lm

debug: CFLAGS = -g -O0 $(WARNINGS) $(MEMORY_CHECKS) $(INCLUDES)
release: CFLAGS = -O2 -DNDEBUG $(WARNINGS) $(INCLUDES)

debug release:
	@$(CC) $(CFLAGS) $(SRC) $(LIBS) -o run

run: debug
	@printf "\n"
	@./run
	@$(MAKE) --no-print-directory clean

clean: 
	@rm -f run
