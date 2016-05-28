#include <stdio.h>
#include "master.h"
#include <string.h>
#include <stdlib.h>

void parse_start(char *buffer) {
    char command[BUFFER_SIZE];
    char computer[BUFFER_SIZE];
    char host[BUFFER_SIZE];
    char path[BUFFER_SIZE];
    char r_port[BUFFER_SIZE];
    char file[BUFFER_SIZE];
    char m_port[BUFFER_SIZE];
    char md[BUFFER_SIZE];
    char garbage[BUFFER_SIZE];

    int rc = sscanf(buffer, "%s %s %s %s %s %s %s %s %s", command, computer, host,
            path, r_port, file, m_port, md, garbage);

    if (rc != 8) {
        fprintf(stderr, "START command incorrect\n");
        return;
    } else {
        printf("Command START\n");
    }
}

void parse_at(char *buffer) {
    char command[BUFFER_SIZE];
    char time_start[BUFFER_SIZE];
    char time_length[BUFFER_SIZE];
    char computer[BUFFER_SIZE];
    char host[BUFFER_SIZE];
    char path[BUFFER_SIZE];
    char r_port[BUFFER_SIZE];
    char file[BUFFER_SIZE];
    char m_port[BUFFER_SIZE];
    char md[BUFFER_SIZE];
    char garbage[BUFFER_SIZE];

    int rc = sscanf(buffer, "%s %s %s %s %s %s %s %s %s %s %s",
                command, time_start, time_length, computer, host,
                path, r_port, file, m_port, md, garbage);

    if (rc != 10) {
        fprintf(stderr, "AT command incorrect\n");
        return;
    } else {
        printf("Command START\n");
    }
}

bool player_with_id_exists(int id) {
    return true;
}

bool is_valid_simple_command(char *buffer) {
    char command[BUFFER_SIZE];
    char id[BUFFER_SIZE];
    char garbage[BUFFER_SIZE];

    int rc = sscanf(buffer, "%s %s %s", command, id, garbage);

    if (rc != 2 || !is_digits_only(id) || !player_with_id_exists(atoi(id))) {
        fprintf(stderr, "%s command incorrect\n", command);
        return false;
    } else {
        printf("Command %s\n", command);
        return true;
    }
}

void parse_play(char *buffer) {
    if (!is_valid_simple_command(buffer)) {
        return;
    }
    // do_play
}

void parse_pause(char *buffer) {
    if (!is_valid_simple_command(buffer)) {
        return;
    }
    // do_pause
}

void parse_title(char *buffer) {
    if (!is_valid_simple_command(buffer)) {
        return;
    }
    // do_title
}

void parse_quit(char *buffer) {
    if (!is_valid_simple_command(buffer)) {
        return;
    }
    // do_quit
}

void parse_telnet_command(char *buffer) {
    char command[BUFFER_SIZE];
    int rc;
    rc = sscanf(buffer, "%s", command);
    if (rc <= 0) {
        fprintf(stderr, "Command wasn't recognized\n");
        return;
    }
    if (!strcmp(command, "START")) {
        parse_start(buffer);
    } else if (!strcmp(command, "AT")) {
        parse_at(buffer);
    } else if (!strcmp(command, "PLAY")) {
        parse_play(buffer);
    } else if (!strcmp(command, "PAUSE")) {
        parse_pause(buffer);
    } else if (!strcmp(command, "TITLE")) {
        parse_title(buffer);
    } else if (!strcmp(command, "QUIT")) {
        parse_quit(buffer);
    } else {
        fprintf(stderr, "Invalid command: %s\n", command);
    }
}