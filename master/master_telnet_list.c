#include <stdlib.h>
#include <stdio.h>
#include "master.h"

static void telnet_list_reset_item(telnet_list *tl, int i) {
    tl->data[i].fd = -1;
    tl->data[i].events = POLLIN;
    tl->data[i].revents = 0;
}

static void telnet_list_grow(telnet_list *tl) {
    tl->max_length *= 2;
    tl->data = realloc(tl->data, sizeof(struct pollfd) * tl->max_length);
    if (!tl->data) {
        syserr("realloc");
    }
    for (int i = tl->length; i < tl->max_length; i++) {
        telnet_list_reset_item(tl, i);
    }
}

static void telnet_list_swap(telnet_list *tl, int i, int j) {
    if (i < 1 || i >= tl->length) {
        return;
    }
    if (j < 1 || j >= tl->length || i == j) {
        return;
    }
    struct pollfd temp = tl->data[i];
    tl->data[i] = tl->data[j];
    tl->data[j] = temp;
}

void telnet_list_initialize(telnet_list *tl) {
    tl->length = 0;
    tl->max_length = LIST_INIT_SIZE;
    tl->data = malloc(sizeof(struct pollfd) * tl->max_length);
    if (!tl->data) {
        syserr("malloc");
    }
    for (int i = 0; i < tl->max_length; i++) {
        telnet_list_reset_item(tl, i);
    }
}

void telnet_list_add(telnet_list *tl, int fd) {
    if (tl->length == tl->max_length) {
        telnet_list_grow(tl);
    }
    tl->data[tl->length].fd = fd;
    tl->length += 1;
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
}

void telnet_list_print(telnet_list *tl) {
    for (int i = 0; i < tl->length; i++) {
        printf("Telnet list item %d: fd: %d\n", i, tl->data[i].fd);
    }
}