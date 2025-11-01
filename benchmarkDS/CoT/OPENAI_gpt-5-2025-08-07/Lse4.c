#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

static const char* VEHICLES[] = {"Car", "Bus", "Truck", "Bike", "Scooter", "Van", "SUV"};
static const size_t VEHICLES_LEN = sizeof(VEHICLES) / sizeof(VEHICLES[0]);

// Accepts the index input as a parameter, returns the vehicle or an error message
const char* get_vehicle_at_index(const char* index_input) {
    static const char* ERR_NULL = "Error: index input is null";
    static const char* ERR_EMPTY = "Error: empty index input";
    static const char* ERR_FORMAT = "Error: invalid index format";
    static const char* ERR_BOUNDS = "Error: index out of bounds";

    if (index_input == NULL) {
        return ERR_NULL;
    }

    const char* start = index_input;
    while (*start && isspace((unsigned char)*start)) start++;
    if (*start == '\0') {
        return ERR_EMPTY;
    }

    errno = 0;
    char* endptr = NULL;
    long val = strtol(start, &endptr, 10);

    if (start == endptr) {
        return ERR_FORMAT;
    }

    while (*endptr && isspace((unsigned char)*endptr)) endptr++;
    if (*endptr != '\0') {
        return ERR_FORMAT;
    }

    if (errno == ERANGE || val < 0 || val > (long)(VEHICLES_LEN - 1)) {
        return ERR_BOUNDS;
    }

    return VEHICLES[(size_t)val];
}

int main(void) {
    const char* tests[] = {"0", "2", "6", "-1", "abc"};
    size_t ntests = sizeof(tests) / sizeof(tests[0]);
    for (size_t i = 0; i < ntests; ++i) {
        const char* res = get_vehicle_at_index(tests[i]);
        printf("Input: \"%s\" -> %s\n", tests[i], res);
    }
    return 0;
}