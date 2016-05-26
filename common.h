#ifndef SIK_CHAT_COMMON_H
#define SIK_CHAT_COMMON_H

#include <stdint.h>
#include <stdbool.h>

#define BUFFER_SIZE 64 * 1024
#define POLL_WAIT_TIME 5000
#define POLL_REVENTS POLLHUP | POLLIN | POLLERR

typedef struct buffer_state {
    ssize_t length_read;
    char buf[BUFFER_SIZE];
} buffer_state;

#endif //SIK_CHAT_COMMON_H
