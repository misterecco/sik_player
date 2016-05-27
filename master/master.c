#include <stdio.h>
#include <stdlib.h>
#include "master.h"

static void validate_number_of_arguments(int argc, char **argv) {
    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv) {
    validate_number_of_arguments(argc, argv);
    exit(EXIT_SUCCESS);
}