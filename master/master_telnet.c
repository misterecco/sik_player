#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <memory.h>
#include "master.h"

void create_central_socket(telnet_list *tl) {
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock< 0) {
        perror("Opening stream socket");
        exit(EXIT_FAILURE);
    }
    telnet_list_add(tl, sock);
}

// TODO: randomize listening port
void bind_port_to_socket(telnet_list *tl, config *c) {
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(c->connection_port);
    if (bind(tl->data[0].fd, (struct sockaddr*)&server,
             (socklen_t) sizeof(server)) < 0) {
        perror("Binding stream socket");
        exit(EXIT_FAILURE);
    }
}

void listen_on_central_socket(telnet_list *tl) {
    if (listen(tl->data[0].fd, 5) == -1) {
        perror("Starting to listen");
        exit(EXIT_FAILURE);
    }
}

void reset_revents(telnet_list *tl) {
    for (int i = 0; i < tl->length; ++i) {
        tl->data[i].revents = 0;
    }
}

void accept_new_client(telnet_list *tl) {
    int msgsock;
    if (tl->data[0].revents & POLLIN) {
        msgsock = accept(tl->data[0].fd, (struct sockaddr*)0, (socklen_t*)0);
        if (msgsock == -1) {
            perror("accept");
        } else {
            telnet_list_add(tl, msgsock);
            printf("Accepted new telnet client, number: %ld\n", tl->length - 1);
        }
    }
}

void close_client_socket(telnet_list *tl, int cn) {
    if (close(tl->data[cn].fd) < 0) {
        perror("close");
    }
    telnet_list_delete(tl, cn);
    printf("Closed telnet connection with client %d\n", cn);
}

// TODO: parse commands and handle them
static void check_client(telnet_list *tl, int cn) {
    ssize_t rval;
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    if (!(tl->data[cn].revents & (POLLIN | POLLERR | POLLHUP))) {
        return;
    }
    rval = read(tl->data[cn].fd, buffer, sizeof(buffer));
    if (rval == 0) {
        close_client_socket(tl, cn);
        return;
    } else if (rval < 0) {
        perror("Reading stream message");
        close_client_socket(tl, cn);
        return;
    }
    printf("Message received from telnet client number %d: %s\n", cn, buffer);
}

void handle_client_messages(telnet_list *tl) {
    for (int i = 1; i < tl->length; ++i) {
        check_client(tl, i);
    }
}
