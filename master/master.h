#ifndef SIK_PLAYER_MASTER_H
#define SIK_PLAYER_MASTER_H

#include <stdbool.h>
#include <poll.h>
#include "../libs/err.h"

#define LIST_INIT_SIZE 2
#define POLL_REFRESH_TIME 3000
#define BUFFER_SIZE 8 * 1024

typedef struct player_state {
    int id; // == port number
    bool is_scheduled;
} player_state;

typedef struct telnet_list {
    size_t length;
    size_t max_length;
    struct pollfd *data;
} telnet_list;

typedef struct config {
    int connection_port;
} config;

// master_initialize
void validate_number_of_arguments(int argc, char **argv);
void initialize_config(config *c);

// master_telnet_list
void telnet_list_initialize(telnet_list *tl);
void telnet_list_add(telnet_list *tl, int fd);
void telnet_list_delete(telnet_list *tl, int index);
void telnet_list_destroy(telnet_list *tl);
void telnet_list_print(telnet_list *tl);

// master_telnet
void create_central_socket(telnet_list *tl);
void bind_port_to_socket(telnet_list *tl, config *c);
void listen_on_central_socket(telnet_list *tl);
void reset_revents(telnet_list *tl);
void accept_new_client(telnet_list *tl);
void close_client_socket(telnet_list *tl, int cn);
void handle_client_messages(telnet_list *tl);

#endif //SIK_PLAYER_MASTER_H