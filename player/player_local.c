#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "player.h"

void open_dump_file(config *c, char *filepath) {
    if (!strcmp(filepath, "-")) {
        return;
    }
    c->dump_fd = open(filepath, O_CREAT | O_WRONLY, 0644);
    if (c->dump_fd <= 0) {
        syserr("open");
    }
}

void close_dump_file(config *c) {
    if (c->dump_fd == 1) {
        return;
    }
    if (close(c->dump_fd)) {
        syserr("close");
    }
}