#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include "common.h"

bool is_digits_only(char *number) {
    for (int i = 0; i < strlen(number); i++) {
        if (!isdigit(number[i])) {
            return false;
        }
    }
    return true;
}

bool is_port_number_valid(char *port_str) {
    if (strlen(port_str) > 5) {
        return false;
    }
    if (!is_digits_only(port_str)) {
        return false;
    }
    int port = atoi(port_str);
    if (port > USHRT_MAX || port <= 0) {
        return false;
    }
    return true;
}
