CC          := gcc
CFLAGS      := -Wall -O2 -std=gnu99 -pthread
LDFLAGS			:= -c -Wall -O2 -std=gnu99 -pthread
TARGETS = master player

all: $(TARGETS)

err.o: err.c err.h

common.o: common.c common.h err.h

player: err.o common.o
	$(CC) $(CFLAGS) -o $@ player.c player.h player_initialize.c player_local.c \
	player_network.c player_parse.c err.o common.o

master: err.o common.o
	$(CC) $(CFLAGS) -o $@  master.c master_commands.c master.h master_initialize.c \
	master_parse.c master_player_list.c master_ssh.c master_telnet.c master_telnet_list.c \
	master_time.c err.o common.o

clean:
	rm -f *.o $(TARGETS)
