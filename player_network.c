#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include "player.h"

#define _GNU_SOURCE

static void validate_read_value(ssize_t lr) {
    if (lr < 0) {
        syserr("read");
    } else if (lr == 0) {
        perror("Host server ended connection\n");
        exit(EXIT_FAILURE);
    }
}

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

void reset_host_buffer(buffer_state *bs) {
    memset(bs->buf, 0, sizeof(bs->buf));
}

void reset_title_buffer(buffer_state *bs) {
    memset(bs->title, 0, sizeof(bs->title));
}

void switch_reading_metadata(config *c, buffer_state *bs) {
    if (!c->get_metadata) {
        return;
    }
    if (bs->reading_metadata) {
        bs->reading_metadata = false;
        bs->length_read = 0;
        bs->to_read = c->to_read;
    } else {
        bs->reading_metadata = true;
        bs->length_read = 0;
        bs->to_read = 1;
        reset_host_buffer(bs);
    }
}

// TODO: write bytes left from synchro into file
void synchronize_metadata(config *c, buffer_state *bs) {
    ssize_t lr = read(c->host_socket,
                      (bs->buf + bs->length_read),
                      BUFFER_SIZE - (size_t) bs->length_read);
    validate_read_value(lr);
    bs->length_read += lr;
    char *metadata = memmem(bs->buf, BUFFER_SIZE, "StreamTitle", 11);
    if (!metadata) {
        return;
    }
    int metadata_len = metadata[-1] * 16;
    int metadata_position = (int) (metadata - bs->buf);
    printf("Metadata found, length: %d, string: %s\n", metadata_len, metadata);
    c->metadata_synchronized = true;
    bs->length_read = bs->length_read - metadata_position - metadata_len;
    memmove(bs->buf,
            metadata + metadata_len,
            (size_t) bs->length_read);
    return;
}

void get_icy_response(config *c, buffer_state *bs) {
    char header_buffer[BUFFER_SIZE];
    ssize_t lr = read(c->host_socket,
                      (bs->buf + bs->length_read),
                      BUFFER_SIZE - (size_t) bs->length_read);
    validate_read_value(lr);
    bs->length_read += lr;
    get_header_from_buffer(bs, header_buffer);
    if (!strcmp(header_buffer, "")) {
        return;
    }
    parse_icy_response(c, bs, header_buffer);
}

void read_metadata_length(config *c, buffer_state *bs) {
    ssize_t lr = read(c->host_socket, &bs->buf, 1);
    validate_read_value(lr);
    bs->to_read = (size_t) bs->buf[0] * 16;
    printf("Metadata length: %ld\n", bs->to_read);
}

void read_metadata(config *c, buffer_state *bs) {
    if (bs->to_read == 0) {
        switch_reading_metadata(c, bs);
        return;
    }
    ssize_t lr = read(c->host_socket,
                      (bs->buf + bs->length_read),
                      bs->to_read - bs->length_read);
    validate_read_value(lr);
    bs->length_read += lr;
    if (bs->to_read == bs->length_read) {
        get_title_from_metadata(bs);
        switch_reading_metadata(c, bs);
    }
}

void get_metadata(config *c, buffer_state *bs) {
    if (bs->to_read == 1) {
        read_metadata_length(c, bs);
        return;
    }
    read_metadata(c, bs);
}

void get_stream(config *c, buffer_state *bs) {
    reset_host_buffer(bs);
    ssize_t lr = read(c->host_socket, &bs->buf, bs->to_read - bs->length_read);
    bs->length_read += lr;
    validate_read_value(lr);
    write(c->dump_fd, &bs->buf, (size_t) lr);
    if (bs->to_read == bs->length_read) {
        switch_reading_metadata(c, bs);
    }
}
