#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include "player.h"

static void set_host_addr_hints(struct addrinfo *host_addr_hints) {
    memset(host_addr_hints, 0, sizeof(struct addrinfo));
    host_addr_hints->ai_flags = 0;
    host_addr_hints->ai_family = AF_INET;
    host_addr_hints->ai_socktype = SOCK_STREAM;
    host_addr_hints->ai_protocol = IPPROTO_TCP;
}

//TODO: validate server name and port
static void get_server_address(char *server_name, char* port,
                               struct addrinfo *host_addr_hints, struct addrinfo **host_addr_result) {
    set_host_addr_hints(host_addr_hints);
    int rc =  getaddrinfo(server_name, port, host_addr_hints, host_addr_result);
    if (rc != 0) {
        fprintf(stderr, "rc=%d\n", rc);
        syserr("getaddrinfo: %s", gai_strerror(rc));
    }
}

static void create_host_socket(config *c) {
    c->host_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (c->host_socket < 0) {
        syserr("socket");
    }
}

void connect_to_server(config *c, char *server_name, char *port) {
    struct addrinfo host_addr_hints, *host_addr_result;
    get_server_address(server_name, port, &host_addr_hints, &host_addr_result);
    create_host_socket(c);
    if (connect(c->host_socket, host_addr_result->ai_addr, host_addr_result->ai_addrlen) != 0) {
        syserr("connect");
    }
    freeaddrinfo(host_addr_result);
}

void send_icy_request(config *c, char *path) {
    char header[10000];
    prepare_icy_request(c, header, path);
    printf("Sent header:\n%s", header);
    if (write(c->host_socket, &header, sizeof(header)) < 0) {
        syserr("write");
    }
}

void reset_host_buffer(buffer_state *b) {
    memset(b->buf, 0, sizeof(b->buf));
    b->length_read = 0;
}

void synchronize_metadata(config *c, buffer_state *bs) {
    return;
}

void get_icy_response(config *c, buffer_state *bs) {
    char header_buffer[BUFFER_SIZE];
    ssize_t read_len = read(c->host_socket,
                            &bs->buf[bs->length_read],
                            BUFFER_SIZE - (size_t) bs->length_read);
    if (read_len < 0) {
        syserr("read");
    }
    bs->length_read += read_len;
    get_header_from_buffer(bs, header_buffer);
    if (!strcmp(header_buffer, "")) {
        return;
    }
    parse_icy_response(c, bs, header_buffer);
}

void get_metadata(config *c, buffer_state *bs) {
    reset_host_buffer(bs);
    bs->length_read = read(c->host_socket, &bs->buf, c->to_read);
    if (bs->length_read < 0) {
        syserr("read");
    }
    char* metadata = strstr(bs->buf, "StreamTitle");
    if (metadata) {
        printf("metadata found: %s\n", metadata);
    }
    write(c->dump_fd, &bs->buf, (size_t) bs->length_read);
}

void get_stream(config *c, buffer_state *bs) {
    reset_host_buffer(bs);
    bs->length_read = read(c->host_socket, &bs->buf, c->to_read);
    if (bs->length_read < 0) {
        syserr("read");
    }
    char* metadata = strstr(bs->buf, "StreamTitle");
    if (metadata) {
        printf("metadata found: %s\n", metadata);
    }
    write(c->dump_fd, &bs->buf, (size_t) bs->length_read);
}
