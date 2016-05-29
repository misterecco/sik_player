#include <stdio.h>
#include <memory.h>
#include <netdb.h>
#include <unistd.h>
#include "master.h"

static void get_ssh_command(char *command, player_args *pa) {
    memset(command, 0, sizeof(command));
    sprintf(command, "ssh %s \"bash -l -c \'player %s %s %s %s %s %s 3>&2 2>&1 1>&3\'\"",
            pa->computer, pa->host, pa->path,
            pa->r_port, pa->file, pa->m_port, pa->md);
}

static void set_host_addr_hints(struct addrinfo *host_addr_hints) {
    memset(host_addr_hints, 0, sizeof(struct addrinfo));
    host_addr_hints->ai_flags = 0;
    host_addr_hints->ai_family = AF_INET;
    host_addr_hints->ai_socktype = SOCK_DGRAM;
    host_addr_hints->ai_protocol = IPPROTO_TCP;
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
    int sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_TCP);
    if (connect(sock, host_addr_result->ai_addr, host_addr_result->ai_addrlen) != 0) {
        perror("connect player socket");
        close(sock);
        sock = -1;
    }
    freeaddrinfo(host_addr_result);
    return sock;
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

int add_player(player_list *pl, player_args *pa, int telnet_id) {
    int sock = bind_player_socket(pa);
    if (sock < 0) {
        return -1;
    }
    return player_list_add(pl, sock, telnet_id);
}

