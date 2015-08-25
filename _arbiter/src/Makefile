CC = gcc
INCLUDE = ../inc
CFLAGS = -g -c -Wall -pthread -I$(INCLUDE)

_HEADER = $(INCLUDE)/*.h
HEADER = $(patsubst %,$(INCLUDE)/%,$(_HEADER))

OBJ_DIR =../obj
_OBJ =  accept_host_connections.o get_host_demands.o process_demands.o send_path_to_host.o queue.o arbiter_main.o
OBJ = $(patsubst %,$(OBJ_DIR)/%,$(_OBJ))

$(OBJ_DIR)/%.o: %.c $(HEADER)
	@$(CC) -c -o $@ $< $(CFLAGS)

server: $(OBJ)
	@$(CC) -pthread -o arbiter $^ -lm
	@mv arbiter ../build/
	@cp ../build/arbiter ../../
