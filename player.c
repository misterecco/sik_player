#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <poll.h>
#include <fcntl.h>
#include "err.h"
#include "common.h"


static int host_socket = 0;
static int master_socket = 0;
static struct addrinfo host_addr_hints, *host_addr_result;
static buffer_state buffer;
static int dump_fd = 1; // default = stdout
static bool get_metadata;
static bool finish = false;

static void validate_arguments(int argc, char **argv) {
    if (argc != 7) {
        fatal("Usage: %s host path r-port file m-port md\n", argv[0]);
        exit(EXIT_FAILURE);
    }
}

//TODO: validate server name and port
static void get_server_address(char *server_name, char* port) {
    memset(&host_addr_hints, 0, sizeof(struct addrinfo));
    host_addr_hints.ai_flags = 0;
    host_addr_hints.ai_family = AF_INET;
    host_addr_hints.ai_socktype = SOCK_STREAM;
    host_addr_hints.ai_protocol = IPPROTO_TCP;

    int rc =  getaddrinfo(server_name, port,
                          &host_addr_hints, &host_addr_result);
    if (rc != 0) {
        fprintf(stderr, "rc=%d\n", rc);
        syserr("getaddrinfo: %s", gai_strerror(rc));
    }
}

static void create_socket() {
    host_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (host_socket < 0) {
        syserr("socket");
    }
}


static void connect_to_server() {
    if (connect(host_socket, host_addr_result->ai_addr, host_addr_result->ai_addrlen) != 0) {
        syserr("connect");
    }
    freeaddrinfo(host_addr_result);
}

static void open_dump_file(char* filepath) {
    if (!strcmp(filepath, "-")) {
        printf("The data will be written to stdout\n");
        return;
    }
    dump_fd = open(filepath,O_CREAT | O_WRONLY, 0644);
    if (dump_fd <= 0) {
        syserr("open");
    }
    printf("File %s created successfully\n", filepath);
}

static void close_dump_file() {
    if (dump_fd == 1) {
        return;
    }
    if (close(dump_fd)) {
        syserr("close");
    }
}

static void set_get_metadata(char* choice) {
    if (!strcmp(choice, "yes")) {
        get_metadata = true;
        printf("Metadata will be sent\n");
        return;
    } else if (!strcmp(choice, "no")) {
        get_metadata = false;
        printf("Metadata will not be sent\n");
        return;
    } else {
        fatal("6th argument incorrect, only yes/no allowed, provided: %s", choice);
    }
}

static void send_header(char* path, bool get_metadata) {
    char header[10000];
    sprintf(header,
        "GET %s HTTP/1.0\r\n"
        "User-Agent: MPlayer 2.0-728-g2c378c7-4build1\r\n"
        "Accept: */*\r\n"
        "Icy-MetaData : %d\r\n"
        "Connection: close\r\n"
        "\r\n",
        path, get_metadata
    );
    printf("Sent header:\n%s", header);
    if (write(host_socket, &header, sizeof(header)) < 0) {
        syserr("write");
    }

}

int main (int argc, char **argv) {
    validate_arguments(argc, argv);
    open_dump_file(argv[4]);
    set_get_metadata(argv[6]);
    get_server_address(argv[1], argv[3]);
    create_socket();
    connect_to_server();
    send_header(argv[2], get_metadata);
    do {
        buffer.length_read = read(host_socket, &buffer.buf, BUFFER_SIZE);
        write(dump_fd, &buffer.buf, (size_t) buffer.length_read);
    } while(!finish);
    close_dump_file();
    exit(EXIT_SUCCESS);
}
