#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include "common.h"

bool is_port_number_valid(char *port_str) {
    if (strlen(port_str) > 5) {
        return false;
    }
    int port = atoi(port_str);
    if (port > USHRT_MAX ||port < 0) {
        return false;
    }
    return true;
}
