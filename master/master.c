#include <stdio.h>
#include <stdlib.h>
#include "master.h"

static telnet_list tl;
static player_list pl;
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

    validate_arguments(argc, argv);
    initialize_config(&c, argc, argv);
    telnet_list_initialize(&tl);
    player_list_initialize(&pl);
    create_central_socket(&tl);
    bind_port_to_socket(&tl, &c);
    listen_on_central_socket(&tl);


    int c = 0;
    do {
        do_poll();
        c++;
    } while (c < 30);

    telnet_list_print(&tl);
    player_list_print(&pl);

    player_list_destroy(&pl);
    telnet_list_destroy(&tl);
    exit(EXIT_SUCCESS);
}