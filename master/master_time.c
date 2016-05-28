#include <time.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "master.h"

static int get_system_time() {
    time_t rawtime;
    struct tm *timeinfo;
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    return timeinfo->tm_hour * 3600 + timeinfo->tm_min * 60 + timeinfo->tm_sec;
}

static bool is_time_string_valid(char *time) {
    if (strlen(time) != 5 || !isdigit(time[0]) || !isdigit(time[1]) ||
        time[2] != '.' || !isdigit(time[3]) || !isdigit(time[4])) {
        return false;
    }
    int t0 = time[0], t3 = time[3];
    if (t3 == '6' || t3 == '7' || t3 == '8' || t3 == '9') {
        return false;
    }
    if (t0 == '0' || t0 == '1' || t0 == '2') {
        return true;
    }
    return false;
}

static int get_time_from_string(char *time) {
    char hours[3];
    strncpy(hours, time, 2);
    char minutes[3];
    strncpy(minutes, time + 3, 2);
    return atoi(hours) * 3600 + atoi(minutes) * 60;
}

int calculate_sleep_time(char *time) {
    if (!is_time_string_valid(time)) {
        return -1;
    }
    int current_time = get_system_time();
    int scheduled_time = get_time_from_string(time);
    if (scheduled_time < current_time) {
        return scheduled_time - current_time + 24 * 3600;
    }
    return scheduled_time - current_time;
}
