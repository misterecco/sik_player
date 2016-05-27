#include <stdio.h>
#include <stdlib.h>
#include "master.h"

static telnet_list tl;
static config c;



static void do_poll() {
    reset_revents(&tl);
    int ret = poll(tl.data, tl.length, POLL_REFRESH_TIME);
    if (ret < 0) {
        perror("poll");
    }
    else if (ret > 0) {
        accept_new_client(&tl);
        handle_client_messages(&tl);
    }
}

int main(int argc, char **argv) {
    validate_number_of_arguments(argc, argv);
    initialize_config(&c);
    telnet_list_initialize(&tl);
    create_central_socket(&tl);
    bind_port_to_socket(&tl, &c);
    listen_on_central_socket(&tl);

    do {
        do_poll();
    } while (true);

    telnet_list_print(&tl);

    telnet_list_destroy(&tl);
    exit(EXIT_SUCCESS);
}