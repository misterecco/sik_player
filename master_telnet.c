#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <memory.h>
#include <time.h>
#include <errno.h>
#include "master.h"


static int get_random_port_number() {
    srand ((unsigned int) time(NULL));
    int number = rand() % 40000;
    return number + 10000;
}

static void bind_given_port(telnet_list *tl, config *c, struct sockaddr_in *server) {
    server->sin_port = htons(c->connection_port);
    if (bind(tl->data[0].fd, (struct sockaddr*)server,
             (socklen_t) sizeof(*server)) < 0) {
        syserr("Binding stream socket");
    }
}

static void bind_random_port(telnet_list *tl, config *c, struct sockaddr_in *server) {
    while(true) {
        c->connection_port = get_random_port_number();
        server->sin_port = htons(c->connection_port);
        if (bind(tl->data[0].fd, (struct sockaddr*)server,
                 (socklen_t) sizeof(*server)) < 0) {
            if (errno != EADDRINUSE) {
                syserr("Binding stream socket");
            }
        } else {
            printf("Listening on port: %d\n", c->connection_port);
            return;
        }
    }
}

void create_central_socket(telnet_list *tl) {
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock< 0) {
        perror("Opening stream socket");
        exit(EXIT_FAILURE);
    }
    telnet_list_add(tl, sock);
}

void bind_port_to_socket(telnet_list *tl, config *c) {
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    if (c->connection_port > 0) {
        bind_given_port(tl, c, &server);
    } else {
        bind_random_port(tl, c, &server);
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

static bool get_line_from_buffer(telnet_list *tl, char* buffer, int cn) {
    memset(buffer, 0, BUFFER_SIZE);
    char* source = tl->state[cn].buffer;
    char* line_end = strstr(source, "\r\n");
    if (!line_end) {
        line_end = strstr(source, "\n");
    }
    if (!line_end) {
        line_end = strstr(source, "\r");
    }
    if (!line_end) {
        return false;
    }
    size_t line_end_position = (size_t) (line_end - source);
    memcpy(buffer, source, line_end_position);
    if (!strncmp(line_end, "\r\n", 2)) {
        memmove(source, line_end + 2, BUFFER_SIZE - line_end_position - 2);
        tl->state[cn].length_read -= line_end_position + 2;
    } else {
        memmove(source, line_end + 1, BUFFER_SIZE - line_end_position - 1);
        tl->state[cn].length_read -= line_end_position + 1;
    }
    return true;
}

static bool is255(char c) {
    if (c + 256 == 255) {
        return true;
    }
    return false;
}

static bool is251254(char c) {
    int val = c + 256;
    if (val >= 251 && val <= 254) {
        return true;
    }
    return false;
}

static void remove_chars_from_buffer(telnet_list *tl, int cn, int start, int length) {
    memmove(tl->state[cn].buffer + start, tl->state[cn].buffer + start + length,
            (size_t)(BUFFER_SIZE - start - length));
    tl->state[cn].length_read -= length;
}

static void purge_steering_sequences(telnet_list *tl, int cn) {
    char* source = tl->state[cn].buffer;
    int i = 0;

    while (i < tl->state[cn].length_read) {
        if (is255(source[i])) {
            if (is255(source[i+1])) {
                remove_chars_from_buffer(tl, cn, i, 1);
                i++;
            } else if (is251254(source[i+1])) {
                remove_chars_from_buffer(tl, cn, i, 3);
            } else {
                remove_chars_from_buffer(tl, cn, i, 2);
            }
        } else {
            i++;
        }
    }
}

static void check_client(telnet_list *tl, player_list *pl, int cn) {
    ssize_t rval;
    memset(tl->state[cn].buffer + tl->state[cn].length_read, 0,
           (size_t) (BUFFER_SIZE - tl->state[cn].length_read));
    if (!(tl->data[cn].revents & (POLLIN | POLLERR | POLLHUP))) {
        return;
    }
    rval = read(tl->data[cn].fd, tl->state[cn].buffer + tl->state[cn].length_read,
                (size_t) (BUFFER_SIZE - tl->state[cn].length_read));
    if (rval == 0) {
        close_client_socket(tl, cn);
        return;
    } else if (rval < 0) {
        perror("Reading stream message");
        close_client_socket(tl, cn);
        return;
    }
    tl->state[cn].length_read += rval;
    purge_steering_sequences(tl, cn);
    char buffer[BUFFER_SIZE];
    while (get_line_from_buffer(tl, buffer, cn)) {
        parse_telnet_command(tl, pl, cn, buffer);
    }
}

void handle_client_messages(telnet_list *tl, player_list *pl) {
    for (int i = 1; i < tl->length; ++i) {
        check_client(tl, pl, i);
    }
}

void send_message_to_client(telnet_list *tl, int telnet_id, char *message) {
    int index = telnet_list_find_by_id(tl, telnet_id);
    if (index < 0) {
        return;
    }
    size_t len = strlen(message);
    ssize_t rc = write(tl->data[index].fd, message, len);
    if (rc < len) {
        perror("write to telnet client");
    }
}
