#include <stdio.h>
#include <memory.h>
#include "master.h"

static void get_ssh_command(char *command, player_args *pa) {
    memset(command, 0, sizeof(command));
    sprintf(command, "ssh %s \"bash -l -c \'player %s %s %s %s %s %s 3>&2 2>&1 1>&3\'\"",
            pa->computer, pa->host, pa->path,
            pa->r_port, pa->file, pa->m_port, pa->md);
}


void run_ssh(player_args *pa) {
    char command[BUFFER_SIZE * 16];
    get_ssh_command(command, pa);
    FILE *fd = popen(command, "r");
    char buffer[BUFFER_SIZE];
    fgets(buffer, BUFFER_SIZE, fd);

    printf("Output: %s\n", buffer);

    printf("Exit status: %d\n", pclose(fd) / 256);
}



