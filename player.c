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
#include <pthread.h>
#include "err.h"
#include "common.h"


static int host_socket = 0;
static int master_socket = 0;
static struct pollfd host_socket_state;
static struct pollfd master_socket_state;
static struct addrinfo host_addr_hints, *host_addr_result;
static buffer_state buffer;
static int dump_fd = 1; // default = stdout
static bool get_metadata;
static bool finish = false;
static bool header_parsed = false;
static pthread_t thread;
static pthread_attr_t attr;

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
        "Icy-MetaData: %d\r\n"
        "Connection: close\r\n"
        "\r\n",
        path, get_metadata
    );
    printf("Sent header:\n%s", header);
    if (write(host_socket, &header, sizeof(header)) < 0) {
        syserr("write");
    }
}

static void reset_host_buffer() {
    memset(buffer.buf, 0, sizeof(buffer.buf));
    buffer.length_read = 0;
}

static void get_header_from_buffer(char* header_buffer) {
    memset(header_buffer, 0, sizeof(header_buffer));
    if (buffer.length_read < 0) {
        syserr("read");
    }
    char* header_end = strstr(buffer.buf, "\r\n\r\n");
    if (!header_end) {
        return;
    }
    strncpy(header_buffer, buffer.buf, header_end - buffer.buf);
    memmove(buffer.buf, header_end + 4, buffer.length_read - strlen(header_buffer) - 4);
}

static int get_status_code(char* header_buffer) {
    char code[4];
    strncpy(code, &header_buffer[4], 3);
    int c = atoi(code);
    printf("Code: %d\n", c);
    return c;
}

static long get_metadata_length(char* header_buffer) {
    char* data_lenght = strstr(header_buffer, "icy-metaint");
    if (data_lenght) {
        printf("Data length found: %s\n", data_lenght);
    } else {
        perror("Data length not found\n");
        finish = true;
        return -1;
    }
    long dl = strtol(&data_lenght[12], NULL, 10);
    return dl;
}

static void parse_http_response() {
    char header_buffer[BUFFER_SIZE];
    buffer.length_read += read(host_socket,
                              &buffer.buf[buffer.length_read],
                              BUFFER_SIZE - (size_t) buffer.length_read);
    get_header_from_buffer(header_buffer);
    if (!strcmp(header_buffer, "")) {
        return;
    }
    printf("header: \n%s\n", header_buffer);
    header_parsed = true;
    int http_response_code = get_status_code(header_buffer);
    if (http_response_code != 200) {
        fprintf(stderr, "Response code form server: %d", http_response_code);
        finish = true;
        return;
    }
    long metadata_length;
    if (get_metadata){
        metadata_length = get_metadata_length(header_buffer);
        printf("Metadata length: %ld\n", metadata_length);
    }
    write(dump_fd, &buffer.buf, (size_t) buffer.length_read);
}

static void get_stream() {
    reset_host_buffer();
    buffer.length_read = read(host_socket, &buffer.buf, BUFFER_SIZE);
    if (buffer.length_read < 0) {
        syserr("read");
    }
    char* metadata = strstr(buffer.buf, "StreamTitle");
    if (metadata) {
        printf("metadata found: %s\n", metadata);
    }
    write(dump_fd, &buffer.buf, (size_t) buffer.length_read);
}

static void initialize_pthread_attr() {
    if (pthread_attr_init(&attr) != 0) {
        syserr("attr init");
    }
    if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) {
        syserr("attr modification");
    }
}

static void destroy_pthread_attr() {
    pthread_attr_destroy(&attr);
}

static void *worker(void *init_data) {
    while(poll(&host_socket_state, 1, POLL_WAIT_TIME)) {
        if (!header_parsed) {
            perror("header not yet parsed\n");
            parse_http_response();
        } else {
            get_stream();
        }
    }
}

static void start_thread() {
    if (pthread_create(&thread, &attr, worker, NULL) != 0) {
        syserr("pthread_create");
    }
}

static void listen_for_master_commands() {
    if (poll(&master_socket_state, 1, POLL_WAIT_TIME)) {
        perror("Message form master arrived\n");
    }
    return;
}

static void initialize_poll() {
    host_socket_state.fd = host_socket;
    host_socket_state.events = POLLIN;
    master_socket_state.fd = master_socket;
    master_socket_state.events = POLLIN;
}

int main (int argc, char **argv) {
    validate_arguments(argc, argv);
    open_dump_file(argv[4]);
    set_get_metadata(argv[6]);
    get_server_address(argv[1], argv[3]);
    create_socket();
    connect_to_server();
    initialize_pthread_attr();
    initialize_poll();
    send_header(argv[2], get_metadata);
    reset_host_buffer();
    start_thread();
    while (!finish) {
        listen_for_master_commands();
    }
    destroy_pthread_attr();
    close_dump_file();
    exit(EXIT_SUCCESS);
}
