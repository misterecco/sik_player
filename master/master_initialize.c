#include <stdio.h>
#include <stdlib.h>
#include "master.h"

void validate_number_of_arguments(int argc, char **argv) {
    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
}

void initialize_config(config *c) {
    c->connection_port = 20160;
}