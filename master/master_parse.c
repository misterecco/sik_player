#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "master.h"

bool validate_start(player_args *pa, char *buffer) {
    char command[BUFFER_SIZE];
    char garbage[BUFFER_SIZE];

    int rc = sscanf(buffer, "%s %s %s %s %s %s %s %s %s", command,
                    pa->computer, pa->host, pa->path, pa->r_port,
                    pa->file, pa->m_port, pa->md, garbage);

    if (rc != 8) {
        fprintf(stderr, "START command incorrect\n");
        return false;
    }
    printf("Command START\n");
    return true;
}

bool validate_at(player_args *pa, char *buffer) {
    char command[BUFFER_SIZE];
    char time_start[BUFFER_SIZE];
    char time_length[BUFFER_SIZE];
    char garbage[BUFFER_SIZE];

    int rc = sscanf(buffer, "%s %s %s %s %s %s %s %s %s %s %s",
                command, time_start, time_length, pa->computer, pa->host,
                pa->path, pa->r_port, pa->file, pa->m_port, pa->md, garbage);

    if (rc != 10) {
        fprintf(stderr, "AT command incorrect\n");
        return false;
    }
    pa->start_time = calculate_sleep_time(time_start);
    if (!is_digits_only(time_length) || pa->start_time < 0
        || atoi(time_length) == 0) {
        return false;
    }
    pa->quit_time = pa->start_time + atoi(time_length) * 60;
    printf("Command AT\n");
    return true;
}

bool player_with_id_exists(player_list *pl, player_args *pa) {
    pa->index = player_list_find_by_id(pl, pa->id);
    if (pa->index < 0) {
        return false;
    }
    return true;
}

static bool is_valid_simple_command(player_list *pl, player_args *pa,
                                    char *buffer) {
    char command[BUFFER_SIZE];
    char id[BUFFER_SIZE];
    char garbage[BUFFER_SIZE];

    int rc = sscanf(buffer, "%s %s %s", command, id, garbage);
    pa->id = atoi(id);

    if (rc != 2 || !is_digits_only(id) || !player_with_id_exists(pl, pa)) {
        fprintf(stderr, "%s command incorrect\n", command);
        return false;
    } else {
        printf("Command %s\n", command);
        return true;
    }
}

static bool has_semicolon(char* buffer) {
    for (int i = 0; i < strlen(buffer); i++) {
        if (buffer[i] == ';') {
            return true;
        }
    }
    return false;
}

void parse_telnet_command(telnet_list *tl, player_list *pl,
                          int telnet_id, char *buffer) {
    char command[BUFFER_SIZE];
    int rc;
    player_args pa;
    pa.telnet_id = telnet_id;
    rc = sscanf(buffer, "%s", command);
    if (rc <= 0 || has_semicolon(buffer)) {
        send_error_to_client_no_id(tl, pl, &pa);
        return;
    }
    if (!strcmp(command, "START")) {
        if (validate_start(&pa, buffer)) {
            start_command(tl, pl, &pa);
        } else {
            send_error_to_client_no_id(tl, pl, &pa);
        }
    } else if (!strcmp(command, "AT")) {
        if (validate_at(&pa, buffer)) {
            at_command(tl, pl, &pa);
        } else {
            send_error_to_client_no_id(tl, pl, &pa);
        }
    } else if (!strcmp(command, "PLAY")) {
        if (is_valid_simple_command(pl, &pa, buffer)
            && pl->data[pa.index].is_running) {
            play_command(tl, pl, &pa);
        } else {
            send_error_to_client(tl, pl, &pa);
        }
    } else if (!strcmp(command, "PAUSE")) {
        if (is_valid_simple_command(pl, &pa, buffer)
            && pl->data[pa.index].is_running) {
            pause_command(tl, pl, &pa);
        } else {
            send_error_to_client(tl, pl, &pa);
        }
    } else if (!strcmp(command, "TITLE")) {
        if (is_valid_simple_command(pl, &pa, buffer)
            && pl->data[pa.index].is_running) {
            title_command(tl, pl, &pa);
        } else {
            send_error_to_client(tl, pl, &pa);
        }
    } else if (!strcmp(command, "QUIT")) {
        if (is_valid_simple_command(pl, &pa, buffer)) {
            quit_command(tl, pl, &pa);
        } else {
            send_error_to_client(tl, pl, &pa);
        }
    } else {
        send_error_to_client_no_id(tl, pl, &pa);
    }
}