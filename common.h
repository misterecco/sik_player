#ifndef SIK_CHAT_COMMON_H
#define SIK_CHAT_COMMON_H

#include <stdint.h>
#include <stdbool.h>

#define BUFFER_SIZE 10000

typedef struct buffer_state {
    ssize_t length_read;
    char buf[BUFFER_SIZE];
} buffer_state;

#endif //SIK_CHAT_COMMON_H
