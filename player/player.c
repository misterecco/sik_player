#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <pthread.h>
#include <unistd.h>
#include "player.h"

static pthread_t thread;
static pthread_attr_t attr;

static buffer_state host_buffer;
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
    if (cfg.get_metadata && host_buffer.reading_metadata) {
        get_metadata(&cfg, &host_buffer);
    } else {
        get_stream(&cfg, &host_buffer);
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
    while(!cfg.finish) {
        reset_host_revents(&ss);
        do_poll();
        if (!cfg.header_parsed) {
            get_icy_response(&cfg, &host_buffer);
        } else {
            main_job();
        }
    }
    return 0;
}

static void start_thread() {
    if (pthread_create(&thread, &attr, worker, NULL) != 0) {
        syserr("pthread_create");
    }
}

static void listen_for_master_commands() {
    int poll_ret = poll(&ss.master, 1, POLL_WAIT_TIME);
    if (poll_ret > 0) {
        get_master_command(&cfg, &host_buffer);
    } else if (poll_ret < 0) {
        syserr("poll");
    }
    return;
}

static void close_sockets(config *c) {
    if (close(c->master_socket) < 0) {
        syserr("close");
    }
    if (close(c->host_socket) < 0) {
        syserr("close");
    }
}

int main (int argc, char **argv) {
    init_config(&cfg);
    validate_arguments_number(argc, argv);
    open_dump_file(&cfg, argv[4]);
    set_get_metadata(&cfg, argv[6]);
    connect_to_server(&cfg, argv[1], argv[3]);
    create_datagram_socket(&cfg);
    bind_datagram_socket(&cfg, atoi(argv[5]));
    initialize_pthread_attr();
    initialize_poll(cfg, &ss);
    send_icy_request(&cfg, argv[2]);
    reset_host_buffer(&host_buffer);
    start_thread();
    while (!cfg.finish) {
//        fprintf(stderr, "some garbage\n");
        listen_for_master_commands();
    }
    destroy_pthread_attr();
    close_dump_file(&cfg);
    close_sockets(&cfg);
    exit(EXIT_SUCCESS);
}
