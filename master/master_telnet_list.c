#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <memory.h>
#include "master.h"

static int last_id = -1;

static int get_next_id() {
    last_id = (last_id + 1) % USHRT_MAX;
    return last_id;
}

static void telnet_list_reset_item(telnet_list *tl, int i) {
    tl->data[i].fd = -1;
    tl->data[i].events = POLLIN;
    tl->data[i].revents = 0;
    tl->state[i].id = -1;
    tl->state[i].length_read = 0;
    memset(tl->state[i].buffer, 0, sizeof(tl->state[i].buffer));
}

static void telnet_list_grow(telnet_list *tl) {
    tl->max_length *= 2;
    tl->data = realloc(tl->data, sizeof(struct pollfd) * tl->max_length);
    tl->state = realloc(tl->state, sizeof(telnet_state) * tl->max_length);
    if (!tl->data) {
        syserr("realloc");
    }
    for (size_t i = tl->length; i < tl->max_length; i++) {
        telnet_list_reset_item(tl, (int) i);
    }
}

static void telnet_list_swap(telnet_list *tl, int i, int j) {
    if (i < 1 || i > tl->length) {
        return;
    }
    if (j < 1 || j > tl->length || i == j) {
        return;
    }
    struct pollfd temp = tl->data[i];
    tl->data[i] = tl->data[j];
    tl->data[j] = temp;
    telnet_state tmp = tl->state[i];
    tl->state[i] = tl->state[j];
    tl->state[j] = tmp;
}

void telnet_list_initialize(telnet_list *tl) {
    tl->length = 0;
    tl->max_length = LIST_INIT_SIZE;
    tl->data = malloc(sizeof(struct pollfd) * tl->max_length);
    tl->state = malloc(sizeof(telnet_state) * tl->max_length);
    if (!tl->data) {
        syserr("malloc");
    }
    for (int i = 0; i < tl->max_length; i++) {
        telnet_list_reset_item(tl, i);
    }
}

int telnet_list_add(telnet_list *tl, int fd) {
    if (tl->length == tl->max_length) {
        telnet_list_grow(tl);
    }
    tl->data[tl->length].fd = fd;
    int id = get_next_id();
    tl->state[tl->length].id = id;
    tl->length += 1;
    return id;
}

void telnet_list_delete(telnet_list *tl, int index) {
    tl->length -= 1;
    telnet_list_reset_item(tl, index);
    for (int i = index; i < tl->length; i++) {
        telnet_list_swap(tl, i ,i + 1);
    }
}

void telnet_list_destroy(telnet_list *tl) {
    free(tl->data);
    free(tl->state);
}

int telnet_list_find_by_id(telnet_list *tl, int id) {
    for (int i = 1; i < tl->length; i++) {
        if (tl->state[i].id == id) {
            return i;
        }
    }
    return -1;
}

void telnet_list_print(telnet_list *tl) {
    printf("TELNET LIST\n");
    for (int i = 0; i < tl->length; i++) {
        printf("Telnet list item %d: fd: %d, id: %d\n", i, tl->data[i].fd, tl->state[i].id);
    }
}