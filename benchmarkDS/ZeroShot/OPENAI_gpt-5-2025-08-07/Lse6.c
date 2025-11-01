#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

const char* get_vehicle(const char* vehicles[], size_t len, long index) {
    if (index < 0 || (size_t)index >= len) {
        return NULL;
    }
    return vehicles[index];
}

void print_vehicles(const char* vehicles[], size_t len) {
    for (size_t i = 0; i < len; ++i) {
        printf("Vehicle[%zu]: %s\n", i, vehicles[i]);
    }
}

int main(void) {
    const char* vehicles[] = { "Car", "Bike", "Truck", "Boat", "Plane" };
    size_t vlen = sizeof(vehicles) / sizeof(vehicles[0]);

    // Optional interactive prompt controlled by environment variable
    if (getenv("RUN_INTERACTIVE")) {
        char buf[128];
        fputs("Enter vehicle index: ", stdout);
        fflush(stdout);
        if (fgets(buf, sizeof(buf), stdin) != NULL) {
            // Strip trailing newline
            size_t n = strlen(buf);
            if (n > 0 && buf[n - 1] == '\n') buf[n - 1] = '\0';

            errno = 0;
            char *end = NULL;
            long val = strtol(buf, &end, 10);
            // Skip trailing whitespace
            while (end && *end != '\0' && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n' || *end == '\f' || *end == '\v')) {
                end++;
            }
            if (errno != 0 || end == buf || (end && *end != '\0') || val < LONG_MIN || val > LONG_MAX) {
                puts("Invalid input. Please enter a valid integer index.");
            } else {
                const char* res = get_vehicle(vehicles, vlen, val);
                if (res) {
                    printf("Selected vehicle: %s\n", res);
                } else {
                    puts("Invalid index.");
                }
            }
        } else {
            puts("No input received.");
        }
    } else {
        puts("RUN_INTERACTIVE not set; skipping interactive prompt.");
    }

    // 5 test cases
    long test_indices[5] = {0, 4, 2, -1, 10};
    for (size_t i = 0; i < 5; ++i) {
        const char* res = get_vehicle(vehicles, vlen, test_indices[i]);
        if (res) {
            printf("Test index %ld: %s\n", test_indices[i], res);
        } else {
            printf("Test index %ld: Invalid index\n", test_indices[i]);
        }
    }

    // Loop and print all vehicles
    print_vehicles(vehicles, vlen);

    return 0;
}