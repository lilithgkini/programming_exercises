CC = gcc
FLAGS = -Wall -Wextra -Iinclude -g

CLN_TARGET = bin/client
CLN_SRC = $(wildcard src/client/*.c)
CLN_OBJ = $(CLN_SRC:src/client/%.c=obj/client/%.o)

SRV_TARGET = bin/server
SRV_SRC = $(wildcard src/server/*.c)
SRV_OBJ = $(SRV_SRC:src/server/%.c=obj/server/%.o)


default: $(SRV_TARGET) $(CLN_TARGET)

test: clean default
	strace ./$(SRV_TARGET) -f file.db -n

$(SRV_TARGET): $(SRV_OBJ)
	$(CC) $(FLAGS) $(SRV_OBJ) -o $@

obj/server/%.o: src/server/%.c
	$(CC) -c $< -o $@ $(FLAGS)

$(CLN_TARGET): $(CLN_OBJ)
	$(CC) $(FLAGS) $(CLN_OBJ) -o $@

obj/client/%.o: src/client/%.c
	$(CC) -c $< -o $@ $(FLAGS)

clean:
	touch file.db
	rm -f obj/server/*.o
	rm -f obj/client/*.o
	rm -f bin/*
	rm file.db
