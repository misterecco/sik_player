#ifndef SIK_PLAYER_PLAYER_H
#define SIK_PLAYER_PLAYER_H

#include <stdbool.h>
#include <poll.h>
#include <stdint.h>
#include <netdb.h>
#include "../libs/err.h"
#include "../libs/common.h"

#define BUFFER_SIZE 128 * 1024
#define TITLE_SIZE 255 * 16
#define POLL_WAIT_TIME 5000
#define POLL_REVENTS POLLHUP | POLLIN | POLLERR

// data structures

typedef struct buffer_state {
    ssize_t length_read;
    size_t to_read;
    char buf[BUFFER_SIZE];
    char title[TITLE_SIZE];
    bool reading_metadata;
} buffer_state;

typedef struct socket_state {
    struct pollfd host;
    struct pollfd master;
} socket_state;

typedef struct config {
    bool get_metadata;
    bool finish;
    bool header_parsed;
    bool is_paused;
    bool thread_finished;
    int host_socket;
    int master_socket;
    int dump_fd;
    size_t to_read;
} config;

// player_initialize
void init_config(config *cfg);
void validate_arguments_number(int argc, char **argv);
void initialize_poll(config c, socket_state *ss);
void set_get_metadata(config *c, char* choice);
void prepare_icy_request(config *c, char *header, char *path);

// player_network
void connect_to_server(config *c, char *server_name, char *port);
void send_icy_request(config *c, char* path);
void reset_host_buffer(buffer_state *bs);
void synchronize_metadata(config *c, buffer_state *bs);
void get_stream(config *c, buffer_state *bs);
void get_metadata(config *c, buffer_state *bs);
void get_icy_response(config *c, buffer_state *bs);

void create_datagram_socket(config *c);
void bind_datagram_socket(config *c, int port);
void get_master_command(config *c, buffer_state *bs);

// player_local
void open_dump_file(config *c, char *filepath);
void close_dump_file(config *c);

// player_parse
void get_header_from_buffer(buffer_state *bs, char *header_buffer);
void parse_icy_response(config *c, buffer_state *bs, char* hb);
void get_title_from_metadata(buffer_state *bs);
void copy_title_to_buffer(buffer_state *bs, char* stream_title);
int parse_master_request(config *c, buffer_state *bs, char* command);

#endif //SIK_PLAYER_PLAYER_H
