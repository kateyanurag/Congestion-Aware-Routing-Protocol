CC = gcc
INCLUDE = ../inc
CFLAGS = -g -c -Wall -pthread -I$(INCLUDE)

_HEADER = $(INCLUDE)/*.h
HEADER = $(patsubst %,$(INCLUDE)/%,$(_HEADER))

OBJ_DIR =../obj
_OBJ =  host_recieve_demands.o host_send_demands.o host_send_packet.o queue.o raw_socket.o host_main.o 
OBJ = $(patsubst %,$(OBJ_DIR)/%,$(_OBJ))

$(OBJ_DIR)/%.o: %.c $(HEADER)
	@$(CC) -c -o $@ $< $(CFLAGS)

host: $(OBJ)
	@$(CC) -pthread -o cperf $^ -lm
	@mv cperf ../build/
	@cp ../build/cperf ../../

.PHONY: clean
clean:
	rm -f *.o *~