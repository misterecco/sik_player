#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <memory.h>
#include <pthread.h>
#include <unistd.h>
#include "master.h"

static int last_id = -1;

static int get_next_id() {
    last_id = (last_id + 1) % USHRT_MAX;
    return last_id;
}

static pthread_mutex_t mutex;

void player_list_lock() {
    if (pthread_mutex_lock(&mutex) != 0) {
        syserr("lock failed");
    }
}

void player_list_unlock() {
    if (pthread_mutex_unlock(&mutex) != 0) {
        syserr("unlock failed");
    }
}

static void player_list_reset_item(player_list *pl, int i) {
    pl->data[i].id = -1;
    pl->data[i].telnet_id = -1;
    pl->data[i].socket = -1;
    pl->data[i].start_thread = 0;
    pl->data[i].quit_thread = 0;
    pl->data[i].title_thread = 0;
    pl->data[i].to_delete = false;
    pl->data[i].is_running = false;
    memset(pl->data[i].computer, 0, sizeof(pl->data[i].computer));
}

static void player_list_grow(player_list *pl) {
    pl->max_length *= 2;
    pl->data = realloc(pl->data, sizeof(player_state) * pl->max_length);
    if (!pl->data) {
        syserr("realloc");
    }
    for (size_t i = pl->length; i < pl->max_length; i++) {
        player_list_reset_item(pl, (int) i);
    }
}

static void player_list_swap(player_list *pl, int i, int j) {
    if (i < 0 || i > pl->length) {
        return;
    }
    if (j < 0 || j > pl->length || i == j) {
        return;
    }
    player_state temp = pl->data[i];
    pl->data[i] = pl->data[j];
    pl->data[j] = temp;
}

int player_list_find_by_id(player_list *pl, int id) {
    for (int i = 0; i < pl->length; i++) {
        if (pl->data[i].id == id) {
            return i;
        }
    }
    return -1;
}

void player_list_initialize(player_list *pl) {
    pl->length = 0;
    pl->max_length = LIST_INIT_SIZE;
    pl->data = malloc(sizeof(player_state) * pl->max_length);
    if (!pl->data) {
        syserr("malloc");
    }
    for (int i = 0; i < pl->max_length; i++) {
        player_list_reset_item(pl, i);
    }
    if (pthread_mutex_init(&mutex, 0) != 0) {
        syserr("mutex init");
    }
}

int player_list_add(player_list *pl, int sock, int telnet_id) {
    player_list_lock();
    if (pl->length == pl->max_length) {
        player_list_grow(pl);
    }
    pl->data[pl->length].socket = sock;
    pl->data[pl->length].telnet_id = telnet_id;
    int id = get_next_id();
    pl->data[pl->length].id = id;
    pl->length += 1;
    player_list_unlock();
    return id;
}

void player_list_delete(player_list *pl, int id) {
    int index = player_list_find_by_id(pl, id);
    if (index < 0) {
        return;
    }
    player_list_reset_item(pl, index);
    pl->length -= 1;
    for (int i = index; i < pl->length; i++) {
        player_list_swap(pl, i ,i + 1);
    }
}

void player_list_destroy(player_list *pl) {
    free(pl->data);
    pthread_mutex_destroy(&mutex);
}

static void player_list_delete_by_index(player_list *pl, int idx) {
    player_list_reset_item(pl, idx);
    pl->length -= 1;
    for (int i = idx; i < pl->length; i++) {
        player_list_swap(pl, i ,i + 1);
    }
}

void player_list_purge_dead_players(player_list *pl) {
    player_list_lock();
    for (ssize_t i = pl->length - 1; i >= 0; i--) {
        if (pl->data[i].to_delete) {
            if (close(pl->data[i].socket)) {
                perror("close");
            }
            pthread_t qtr = pl->data[i].quit_thread;
            if (qtr) {
                pthread_cancel(qtr);
            }
            player_list_delete_by_index(pl, (int) i);
        }
    }
    player_list_unlock();
}

void player_list_print(player_list *pl) {
    printf("PLAYER LIST\n");
    for (int i = 0; i < pl->length; i++) {
        printf("Player list item %d: id: %d, socket: %d, telnet_id: %d, start_thread: %ld\n",
               i, pl->data[i].id, pl->data[i].socket, pl->data[i].telnet_id,
                pl->data[i].start_thread
        );
    }
}

