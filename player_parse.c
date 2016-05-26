#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include "player.h"


static int get_status_code(char* header_buffer) {
    char code[4];
    strncpy(code, &header_buffer[4], 3);
    return atoi(code);
}

static size_t get_data_length(config *c, char *header_buffer) {
    char* data_length = strstr(header_buffer, "icy-metaint");
    if (!data_length) {
        perror("Server didn't sent icy-metaint value\n");
        exit(EXIT_FAILURE);
    }
    printf("Data length found: %s\n", data_length);
    long dl = atoi(&data_length[12]);
    return (size_t) dl;
}

void get_header_from_buffer(buffer_state *bs, char *header_buffer) {
    memset(header_buffer, 0, sizeof(header_buffer));
    char* header_end = strstr(bs->buf, "\r\n\r\n");
    if (!header_end) {
        return;
    }
    strncpy(header_buffer, bs->buf, header_end - bs->buf);
    memmove(bs->buf, header_end + 4, bs->length_read - strlen(header_buffer) - 4);
    bs->length_read -= strlen(header_buffer) + 4;
}

void parse_icy_response(config *c, buffer_state *bs, char* hb) {
    printf("header: \n%s\n", hb);
    c->header_parsed = true;
    int http_response_code = get_status_code(hb);
    if (http_response_code != 200) {
        fprintf(stderr, "Error: Response code form server: %d\n", http_response_code);
        exit(EXIT_FAILURE);
    }
    if (c->get_metadata) {
        c->to_read = get_data_length(c, hb);
        bs->to_read = c->to_read;
        bs->reading_metadata = false;
        printf("Metadata length: %ld\n", c->to_read);
    }
}

// TODO: parse metadata and save title in appropriate buffer
void get_title_from_metadata(buffer_state *bs) {
    printf("gettitle input %s\n", bs->buf);
    char *metadata = strstr(bs->buf, "StreamTitle");
    if (metadata) {
        printf("Metadata found: %s\n", metadata);
    }
}