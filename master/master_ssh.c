#include <stdio.h>
#include <memory.h>
#include "master.h"

static void get_ssh_command(char *command, player_args *pa) {
    memset(command, 0, BUFFER_SIZE * 16);
    sprintf(command, "ssh -o BatchMode=yes %s \"bash -l -c \'player %s %s %s %s %s %s 3>&2 2>&1 1>&3\'\"",
            pa->computer, pa->host, pa->path,
            pa->r_port, pa->file, pa->m_port, pa->md);
}

void run_ssh(telnet_list *tl, player_args *pa) {
    char command[BUFFER_SIZE * 16];
    get_ssh_command(command, pa);
    FILE *fd = popen(command, "r");
    char buffer[BUFFER_SIZE + 8];
    sprintf(buffer, "ERROR %d: ", pa->id);
    while(fgets(buffer + strlen(buffer), BUFFER_SIZE, fd)) {
        printf("Output: %s", buffer);
        send_message_to_client(tl, pa->telnet_id, buffer);
        memset(buffer, 0, BUFFER_SIZE + 8);
        sprintf(buffer, "ERROR %d: ", pa->id);
    }
    sprintf(buffer, "Player %d ended with status: %d\n", pa->id, pclose(fd) / 256);
    send_message_to_client(tl, pa->telnet_id, buffer);
}
