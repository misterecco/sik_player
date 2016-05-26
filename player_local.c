#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "player.h"

void open_dump_file(config *c, char *filepath) {
    if (!strcmp(filepath, "-")) {
        printf("The data will be written to stdout\n");
        return;
    }
    c->dump_fd = open(filepath, O_CREAT | O_WRONLY, 0644);
    if (c->dump_fd <= 0) {
        syserr("open");
    }
    printf("File %s created successfully\n", filepath);
}

void close_dump_file(config *c) {
    if (c->dump_fd == 1) {
        return;
    }
    if (close(c->dump_fd)) {
        syserr("close");
    }
}