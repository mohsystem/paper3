#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

static const char* VEHICLES[] = {
    "Car", "Bike", "Truck", "Bus", "Scooter",
    "Van", "Train", "Boat", "Plane", "Tractor"
};
static const size_t VEHICLES_COUNT = sizeof(VEHICLES) / sizeof(VEHICLES[0]);
#define MAX_INPUT_LEN 20

// Returns the vehicle at the given index as a const char* (or an error message)
const char* get_vehicle_by_index(const char* input) {
    if (input == NULL) {
        return "Invalid input";
    }

    // Bounded length check
    size_t len = 0;
    while (len <= MAX_INPUT_LEN && input[len] != '\0') {
        len++;
    }
    if (len == 0 || len > MAX_INPUT_LEN) {
        return "Invalid input";
    }

    // Trim leading whitespace (strtol will also skip it, but we will use it for format checks)
    const char* s = input;
    while (*s && isspace((unsigned char)*s)) s++;

    // Ensure we have something after trimming
    if (*s == '\0') {
        return "Invalid input";
    }

    // Validate numeric format: optional sign followed by digits only (until optional trailing WS)
    const char* p = s;
    if (*p == '+' || *p == '-') {
        p++;
        if (*p == '\0') return "Invalid input";
    }
    int has_digit = 0;
    const char* digits_start = p;
    while (*p && isdigit((unsigned char)*p)) {
        has_digit = 1;
        p++;
        // Limit digits to avoid extremely long numbers (already bounded by MAX_INPUT_LEN)
    }
    if (!has_digit) {
        return "Invalid input";
    }
    // Allow trailing whitespace only
    const char* rest = p;
    while (*rest && isspace((unsigned char)*rest)) rest++;
    if (*rest != '\0') {
        return "Invalid input";
    }

    // Parse number
    errno = 0;
    char* endptr = NULL;
    long val = strtol(s, &endptr, 10);
    if (errno == ERANGE) {
        return "Invalid input";
    }
    // endptr may point to trailing whitespace; ensure nothing non-space after
    while (endptr && *endptr && isspace((unsigned char)*endptr)) endptr++;
    if (endptr && *endptr != '\0') {
        return "Invalid input";
    }

    if (val < 0 || (unsigned long)val >= VEHICLES_COUNT) {
        return "Index out of range";
    }

    return VEHICLES[(size_t)val];
}

int main(void) {
    const char* tests[5] = {"0", "3", "10", "-1", "2"};
    for (int i = 0; i < 5; i++) {
        const char* input = tests[i];
        const char* result = get_vehicle_by_index(input);
        printf("Input: %s -> Output: %s\n", input, result);
    }
    return 0;
}