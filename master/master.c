#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "master.h"

static pthread_attr_t attr;

static telnet_list tl;
static player_list pl;
static config c;

static typedef struct thread_data {
    telnet_list *tl;
    player_list *pl;
    player_args pa;
    int sleep_time;
    int id;
} thread_data;

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

static void fill_thread_data(thread_data **data, telnet_list *tl, player_list *pl,
                             player_args *pa, int id, int sleep_time) {
    *data = (thread_data *) malloc(sizeof(thread_data));
    if (*data == NULL) {
        perror("malloc");
        return;
    }
    (*data)->tl = tl;
    (*data)->pl = pl;
    (*data)->pa = *pa;
    (*data)->id = id;
    (*data)->sleep_time = sleep_time;
}

void *start_thread(void *init_data) {
    thread_data data = *(thread_data *) init_data;
    free(init_data);
    sleep((unsigned int) data.sleep_time);
    run_ssh(&data.pa);
    return 0;
}

void *quit_thread(void *init_data) {
    thread_data data = *(thread_data *) init_data;
    free(init_data);
    sleep((unsigned int) data.sleep_time);
    // do_quit
    return 0;
}

void *title_thread(void *init_data) {
    thread_data data = *(thread_data *) init_data;
    free(init_data);
    // do_title
    return 0;
}

// TODO: handle failed pthread creation
void run_thread(telnet_list *tl, player_list *pl,
                  player_args *pa, int id, int sleep_time,  void *(*start_routine) (void *)) {
    thread_data *data;
    fill_thread_data(&data, tl, pl, pa, id, sleep_time);
    if (data == NULL) {
        return;
    }
    if (pthread_create(NULL, &attr, start_routine, (void *)data) != 0) {
        perror("pthread_create");
    }
}

static void do_poll() {
    reset_revents(&tl);
    int ret = poll(tl.data, tl.length, POLL_REFRESH_TIME);
    if (ret < 0) {
        perror("poll");
    }
    else if (ret > 0) {
        accept_new_client(&tl);
        handle_client_messages(&tl, &pl);
    }
}

int main(int argc, char **argv) {

    validate_arguments(argc, argv);
    initialize_config(&c, argc, argv);
    telnet_list_initialize(&tl);
    player_list_initialize(&pl);
    create_central_socket(&tl);
    bind_port_to_socket(&tl, &c);
    listen_on_central_socket(&tl);
    initialize_pthread_attr();

    int c = 0;
    do {
        do_poll();
        c++;
    } while (c < 30);

    telnet_list_print(&tl);
    player_list_print(&pl);

    destroy_pthread_attr();
    player_list_destroy(&pl);
    telnet_list_destroy(&tl);
    exit(EXIT_SUCCESS);
}