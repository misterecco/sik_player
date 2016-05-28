#include <stdio.h>
#include "master.h"

void run_ssh() {
    FILE *fd = popen("ssh localhost \"bash -l -c \'player ant-waw-01.cdn.eurozet.pl / 8602 test5.mp3 50000 yes 3>&2 2>&1 1>&3\'\"", "r");
    char buffer[BUFFER_SIZE];
    fgets(buffer, BUFFER_SIZE, fd);

    printf("Output: %s\n", buffer);

    printf("Exit status: %d\n", pclose(fd) / 256);
}
