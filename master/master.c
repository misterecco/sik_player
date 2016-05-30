#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "master.h"

static pthread_attr_t attr;

static telnet_list tl;
static player_list pl;
static config c;

typedef struct thread_data {
    telnet_list *tl;
    player_list *pl;
    player_args pa;
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

static bool fill_thread_data(thread_data **data, telnet_list *tl, player_list *pl,
                             player_args *pa) {
    *data = (thread_data *) malloc(sizeof(thread_data));
    if (*data == NULL) {
        perror("malloc");
        return true;
    }
    printf("Filling thread data\n");
    (*data)->tl = tl;
    (*data)->pl = pl;
    (*data)->pa = *pa;
    return false;
}

static void *start_thread(void *init_data) {
    thread_data data = *(thread_data *) init_data;
    free(init_data);
    printf("Start thread going to sleep\n");
    sleep((unsigned int) data.pa.start_time);
    printf("Starting player\n");
    run_ssh(data.tl, &data.pa);
    int idx = player_list_find_by_id(data.pl, data.pa.id);
    data.pl->data[idx].start_thread = 0;
    return 0;
}

static void *quit_thread(void *init_data) {
    thread_data data = *(thread_data *) init_data;
    free(init_data);
    printf("Quit thread going to sleep\n");
    sleep((unsigned int) data.pa.quit_time);
    printf("Quitting player\n");
    do_quit(data.tl, data.pl, &data.pa);
    int idx = player_list_find_by_id(data.pl, data.pa.id);
    data.pl->data[idx].quit_thread = 0;
    return 0;
}

static void *title_thread(void *init_data) {
    thread_data data = *(thread_data *) init_data;
    free(init_data);
    do_title(data.tl, data.pl, &data.pa);
    int idx = player_list_find_by_id(data.pl, data.pa.id);
    data.pl->data[idx].title_thread = 0;
    return 0;
}

// TODO: handle failed pthread creation
bool run_start_thread(telnet_list *tl, player_list *pl, player_args *pa) {
    thread_data *data;
    if (fill_thread_data(&data, tl, pl, pa)) {
        fprintf(stderr, "Thread data is null\n");
        return false;
    }
    printf("running START thread\n");
    if (pthread_create(&(pl->data[pa->index].start_thread), &attr,
                       start_thread, (void *)data) != 0) {
        perror("pthread_create");
        return false;
    }
    return true;
}

bool run_at_thread(telnet_list *tl, player_list *pl, player_args *pa) {
    thread_data *data;
    if (fill_thread_data(&data, tl, pl, pa)) {
        fprintf(stderr, "Thread data is null\n");
        return false;
    }
    printf("running AT-START thread\n");
    if (pthread_create(&(pl->data[pa->index].start_thread), &attr,
                       start_thread, (void *)data) != 0) {
        perror("pthread_create");
        return false;
    }
    printf("running AT-QUIT thread\n");
    if (pthread_create(&(pl->data[pa->index].quit_thread), &attr,
                       quit_thread, (void *)data) != 0) {
        perror("pthread_create");
        return false;
    }
    return true;
}

bool run_title_thread(telnet_list *tl, player_list *pl, player_args *pa) {
    thread_data *data;
    if (fill_thread_data(&data, tl, pl, pa)) {
        fprintf(stderr, "Thread data is null\n");
        return false;
    }
    printf("running TITLE thread\n");
    if (pthread_create(&(pl->data[pa->index].title_thread), &attr,
                       title_thread, (void *)data) != 0) {
        perror("pthread_create");
        return false;
    }
    return true;
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

    do {
        do_poll();
    } while (true);

    telnet_list_print(&tl);
    player_list_print(&pl);

    destroy_pthread_attr();
    player_list_destroy(&pl);
    telnet_list_destroy(&tl);
    exit(EXIT_SUCCESS);
}