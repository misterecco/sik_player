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

    validate_arguments(argc, argv);

    char bufferAT[] = "AT 10.10 50 localhost "
            "ant-waw-01.cdn.eurozet.pl / 8600 test5.mp3 50000 yes";
    char bufferSTART[] = "START localhost "
            "ant-waw-01.cdn.eurozet.pl / 8600 test5.mp3 50000 yes";
    char bufferPLAY[] = "PLAY 100";
    char bufferPAUSE[] = "PAUSE 100";
    char bufferTITLE[] = "TITLE 100";
    char bufferQUIT[] = "QUIT 100";
//    parse_telnet_command(bufferAT);
//    parse_telnet_command(bufferSTART);
//    parse_telnet_command(bufferPLAY);
//    parse_telnet_command(bufferPAUSE);
//    parse_telnet_command(bufferTITLE);
//    parse_telnet_command(bufferQUIT);
//    parse_telnet_command("fdsfhdasgh");

//    exit(EXIT_SUCCESS);

    initialize_config(&c, argc, argv);
    telnet_list_initialize(&tl);
    create_central_socket(&tl);
    bind_port_to_socket(&tl, &c);
    listen_on_central_socket(&tl);

//    run_ssh();

    int c = 0;
    do {
        do_poll();
        c++;
    } while (c < 30);

    telnet_list_print(&tl);

    telnet_list_destroy(&tl);
    exit(EXIT_SUCCESS);
}