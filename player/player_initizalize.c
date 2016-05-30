#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "player.h"


void init_config(config *cfg) {
    cfg->get_metadata = false;
    cfg->finish = false;
    cfg->header_parsed = false;
    cfg->is_paused = false;
    cfg->thread_finished = false;
    cfg->host_socket = 0;
    cfg->master_socket = 0;
    cfg->dump_fd = 1;
    cfg->to_read = BUFFER_SIZE;
};

void validate_arguments_number(int argc, char **argv) {
    if (argc != 7 || !is_port_number_valid(argv[3]) || !is_port_number_valid(argv[5])) {
        fatal("Usage: %s host path r-port file m-port md\n", argv[0]);
    }
}

void initialize_poll(config c, socket_state *ss) {
    ss->host.fd = c.host_socket;
    ss->host.events = POLLIN;
    ss->master.fd = c.master_socket;
    ss->master.events = POLLIN;
}

void set_get_metadata(config *c, char* choice) {
    if (!strcmp(choice, "yes")) {
        c->get_metadata = true;
        return;
    } else if (!strcmp(choice, "no")) {
        c->get_metadata = false;
        return;
    } else {
        fatal("6th argument incorrect, only yes/no allowed, provided: %s", choice);
        exit(EXIT_FAILURE);
    }
}

void prepare_icy_request(config *c, char *header, char *path) {
    sprintf(header,
            "GET %s HTTP/1.0\r\n"
                    "Accept: */*\r\n"
                    "Icy-MetaData: %d\r\n"
                    "Connection: close\r\n"
                    "\r\n",
            path, c->get_metadata
    );
}
