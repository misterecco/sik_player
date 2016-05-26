#ifndef SIK_PLAYER_PLAYER_H
#define SIK_PLAYER_PLAYER_H

#include <stdbool.h>
#include <poll.h>
#include <netdb.h>
#include "err.h"
#include "common.h"

// data structures

typedef struct socket_state {
    struct pollfd host;
    struct pollfd master;
} socket_state;

typedef struct config {
    bool get_metadata;
    bool finish;
    bool header_parsed;
    bool is_paused;
    bool metadata_synchronized;
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

// player_local
void open_dump_file(config *c, char *filepath);
void close_dump_file(config *c);

// player_parse
void get_header_from_buffer(buffer_state *bs, char *header_buffer);
void parse_icy_response(config *c, buffer_state *bs, char* hb);
void get_title_from_metadata(buffer_state *bs);

#endif //SIK_PLAYER_PLAYER_H
