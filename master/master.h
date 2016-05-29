#ifndef SIK_PLAYER_MASTER_H
#define SIK_PLAYER_MASTER_H

#include <stdbool.h>
#include <poll.h>
#include <pthread.h>
#include "../libs/err.h"
#include "../libs/common.h"

#define LIST_INIT_SIZE 2
#define POLL_REFRESH_TIME 3000
#define BUFFER_SIZE 8 * 1024

typedef struct player_state {
    int id;
    int telnet_id;
    int socket;
    char computer[BUFFER_SIZE];
    pthread_t start_thread;
    pthread_t quit_thread;
    pthread_t title_thread;
} player_state;

typedef struct player_args {
    char computer[BUFFER_SIZE];
    char host[BUFFER_SIZE];
    char path[BUFFER_SIZE];
    char r_port[BUFFER_SIZE];
    char file[BUFFER_SIZE];
    char m_port[BUFFER_SIZE];
    char md[BUFFER_SIZE];
    int id;
    int telnet_id;
} player_args;

typedef struct player_list {
    size_t length;
    size_t max_length;
    player_state *data;
} player_list;

typedef struct telnet_state {
    int id;
    int length_read;
    char buffer[BUFFER_SIZE];
} telnet_state;

typedef struct telnet_list {
    size_t length;
    size_t max_length;
    struct pollfd *data;
    telnet_state *state;
} telnet_list;

typedef struct config {
    int connection_port;
} config;

// master
void *start_thread(void *init_data);
void *quit_thread(void *init_data);
void *title_thread(void *init_data);
void run_thread(telnet_list *tl, player_list *pl,
                player_args *pa, int id, int sleep_time,
                void *(*start_routine) (void *));

// master_initialize
void validate_arguments(int argc, char **argv);
void initialize_config(config *c, int argc, char **argv);

// master_telnet_list
void telnet_list_initialize(telnet_list *pl);
int telnet_list_add(telnet_list *tl, int fd);
void telnet_list_delete(telnet_list *tl, int index);
void telnet_list_destroy(telnet_list *tl);
void telnet_list_print(telnet_list *tl);
int telnet_list_find_by_id(telnet_list *tl, int id);

// master_player_list
void player_list_initialize(player_list *pl);
int player_list_find_by_id(player_list *pl, int id);
int player_list_add(player_list *pl, int sock, int telnet_id);
void player_list_delete(player_list *pl, int id);
void player_list_destroy(player_list *pl);
void player_list_print(player_list *pl);

// master_telnet
void create_central_socket(telnet_list *tl);
void bind_port_to_socket(telnet_list *tl, config *c);
void listen_on_central_socket(telnet_list *tl);
void reset_revents(telnet_list *tl);
void accept_new_client(telnet_list *tl);
void close_client_socket(telnet_list *tl, int cn);
void handle_client_messages(telnet_list *tl, player_list *pl);
void send_message_to_client(telnet_list *tl, int telnet_id, char *message);

// master_ssh
void run_ssh(telnet_list *tl, player_args *pa, int telnet_id);

// master_time
int calculate_sleep_time(char *time);

// master_parse
void parse_telnet_command(telnet_list *tl, player_list *pl,
                          int telnet_id, char *buffer);

// master_commands
void start_command(telnet_list *tl, player_list *pl, player_args *pa, int telnet_id);
void at_command(telnet_list *tl, player_list *pl, player_args *pa, int telnet_id, int ts, int tq);
void do_quit(player_list *pl, int id);
void do_pause(player_list *pl, int id);
void do_play(player_list *pl, int id);
void do_title(player_list *pl, int id);

#endif //SIK_PLAYER_MASTER_H
