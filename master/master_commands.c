#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <memory.h>
#include "master.h"

static void set_host_addr_hints(struct addrinfo *host_addr_hints) {
    memset(host_addr_hints, 0, sizeof(struct addrinfo));
    host_addr_hints->ai_flags = 0;
    host_addr_hints->ai_family = AF_INET;
    host_addr_hints->ai_socktype = SOCK_DGRAM;
    host_addr_hints->ai_protocol = IPPROTO_UDP;
}

static void get_server_address(char *server_name, char* port,
                               struct addrinfo *host_addr_hints, struct addrinfo **host_addr_result) {
    set_host_addr_hints(host_addr_hints);
    int rc =  getaddrinfo(server_name, port, host_addr_hints, host_addr_result);
    if (rc != 0) {
        syserr("getaddrinfo: %s", gai_strerror(rc));
    }
}

static int bind_player_socket(player_args *pa) {
    struct addrinfo host_addr_hints, *host_addr_result;
    get_server_address(pa->computer, pa->m_port, &host_addr_hints, &host_addr_result);
    int sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (connect(sock, host_addr_result->ai_addr, host_addr_result->ai_addrlen) != 0) {
        perror("connect player socket");
        close(sock);
        sock = -1;
    }
    freeaddrinfo(host_addr_result);
    return sock;
}


static int add_player(player_list *pl, player_args *pa, int telnet_id) {
    int sock = bind_player_socket(pa);
    if (sock < 0) {
        return -1;
    }
    return player_list_add(pl, sock, telnet_id);
}

// TODO: handle failure
void start_command(telnet_list *tl, player_list *pl, player_args *pa, int telnet_id) {
    int id = add_player(pl, pa, telnet_id);
    run_thread(tl, pl, pa, id, 0, start_thread);
    char message[10];
    memset(message, 0, sizeof(message));
    sprintf(message, "OK %d", id);
    send_message_to_client(tl, telnet_id, message);
}

void at_command() {

}