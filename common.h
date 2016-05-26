#ifndef SIK_CHAT_COMMON_H
#define SIK_CHAT_COMMON_H

#include <stdint.h>
#include <stdbool.h>

#define BUFFER_SIZE 128 * 1024
#define TITLE_SIZE 255 * 16
#define POLL_WAIT_TIME 5000
#define POLL_REVENTS POLLHUP | POLLIN | POLLERR

typedef struct buffer_state {
    ssize_t length_read;
    ssize_t to_read;
    char buf[BUFFER_SIZE];
    char title[TITLE_SIZE];
    bool reading_metadata;
} buffer_state;

#endif //SIK_CHAT_COMMON_H
