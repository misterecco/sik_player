#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <pthread.h>
#include "player.h"


static pthread_t thread;
static pthread_attr_t attr;

static buffer_state buffer;
static config cfg;
static socket_state ss;

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

static void reset_host_revents(socket_state *ss) {
    ss->host.revents = 0;
}

static void main_job() {
    if (buffer.reading_metadata) {
        get_metadata(&cfg, &buffer);
    } else {
        get_stream(&cfg, &buffer);
    }
}

static void initialize_stream() {
    if (!cfg.header_parsed) {
        get_icy_response(&cfg, &buffer);
    }
    if (!cfg.metadata_synchronized) {
        synchronize_metadata(&cfg, &buffer);
    }
}

static void do_poll() {
    int poll_ret = poll(&ss.host, 1, POLL_WAIT_TIME);
    if (poll_ret < 0) {
        syserr("poll");
    } else if (poll_ret == 0) {
        perror("Host server timeout (5s)\n");
        exit(EXIT_FAILURE);
    }
}

static void *worker(void *init_data) {
    while(true) {
        reset_host_revents(&ss);
        do_poll();
        if (!cfg.header_parsed || !cfg.metadata_synchronized) {
            initialize_stream();
        } else {
            main_job();
        }
    }
}

static void start_thread() {
    if (pthread_create(&thread, &attr, worker, NULL) != 0) {
        syserr("pthread_create");
    }
}

//TODO: move to network file
static void listen_for_master_commands() {
    if (poll(&ss.master, 1, POLL_WAIT_TIME)) {
        perror("Message form master arrived\n");
    }
    return;
}

int main (int argc, char **argv) {
    init_config(&cfg);
    validate_arguments_number(argc, argv);
    open_dump_file(&cfg, argv[4]);
    set_get_metadata(&cfg, argv[6]);
    connect_to_server(&cfg, argv[1], argv[3]);
    initialize_pthread_attr();
    initialize_poll(cfg, &ss);
    send_icy_request(&cfg, argv[2]);
    reset_host_buffer(&buffer);
    start_thread();
    while (!cfg.finish) {
        listen_for_master_commands();
    }
    destroy_pthread_attr();
    close_dump_file(&cfg);
    exit(EXIT_SUCCESS);
}
