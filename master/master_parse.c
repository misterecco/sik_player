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

// TODO: player should not run for less than 1 minut
bool validate_at(player_args *pa, char *buffer, int *ts, int *tq) {
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
    *ts = calculate_sleep_time(time_start);
    if (!is_digits_only(time_length) || *ts < 0) {
        return false;
    }
    *tq = *ts + atoi(time_length) * 60;
    printf("Command AT\n");
    return true;
}

bool player_with_id_exists(player_list *pl, int id) {
    int idx = player_list_find_by_id(pl, id);
    if (idx < 0) {
        return false;
    }
    return true;
}

static bool is_valid_simple_command(player_list *pl, char *buffer) {
    char command[BUFFER_SIZE];
    char id[BUFFER_SIZE];
    char garbage[BUFFER_SIZE];

    int rc = sscanf(buffer, "%s %s %s", command, id, garbage);

    if (rc != 2 || !is_digits_only(id) || !player_with_id_exists(pl, atoi(id))) {
        fprintf(stderr, "%s command incorrect\n", command);
        return false;
    } else {
        printf("Command %s\n", command);
        return true;
    }
}

static int get_id_from_simple_command(char *buffer) {
    char command[BUFFER_SIZE];
    char id[BUFFER_SIZE];

    sscanf(buffer, "%s %s", command, id);
    return atoi(id);
}

int validate_play(player_list *pl, char *buffer) {
    if (!is_valid_simple_command(pl, buffer)) {
        return -1;
    }
    return get_id_from_simple_command(buffer);
}

int validate_pause(player_list *pl, char *buffer) {
    if (!is_valid_simple_command(pl, buffer)) {
        return -1;
    }
    return get_id_from_simple_command(buffer);
}

int validate_title(player_list *pl, char *buffer) {
    if (!is_valid_simple_command(pl, buffer)) {
        return -1;
    }
    return get_id_from_simple_command(buffer);
}

int validate_quit(player_list *pl, char *buffer) {
    if (!is_valid_simple_command(pl, buffer)) {
        return -1;
    }
    return get_id_from_simple_command(buffer);
}

void parse_telnet_command(telnet_list *tl, player_list *pl,
                          int telnet_id, char *buffer) {
    char command[BUFFER_SIZE];
    int rc;
    rc = sscanf(buffer, "%s", command);
    if (rc <= 0) {
        fprintf(stderr, "Command wasn't recognized\n");
        return;
    }
    player_args pa;
    if (!strcmp(command, "START")) {
        if (validate_start(&pa, buffer)) {
            start_command(tl, pl, &pa, telnet_id);
        } else {
            // fail
        }
    } else if (!strcmp(command, "AT")) {
        int start_time, quit_time;
        if (validate_at(&pa, buffer, &start_time, &quit_time)) {
            at_command(tl, pl, &pa, telnet_id, start_time, quit_time);
        } else {
            // fail
        }
    } else if (!strcmp(command, "PLAY")) {
        validate_play(pl, buffer);
    } else if (!strcmp(command, "PAUSE")) {
        validate_pause(pl, buffer);
    } else if (!strcmp(command, "TITLE")) {
        validate_title(pl, buffer);
    } else if (!strcmp(command, "QUIT")) {
        validate_quit(pl, buffer);
    } else {
        fprintf(stderr, "Invalid command: %s\n", command);
    }
}