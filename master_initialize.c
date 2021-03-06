#include <stdio.h>
#include <stdlib.h>
#include "master.h"


void validate_arguments(int argc, char **argv) {
    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if (argc == 2 && !is_port_number_valid(argv[1])) {
        fprintf(stderr, "Invalid port number\n");
        exit(EXIT_FAILURE);
    }
}

void initialize_config(config *c, int argc, char **argv) {
    if (argc == 2) {
        c->connection_port = atoi(argv[1]);
    } else {
        c->connection_port = -1;
    }
    c->finish = false;
}