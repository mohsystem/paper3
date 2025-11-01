#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

const char* get_vehicle(const char* vehicles[], size_t len, long index) {
    if (index >= 0 && (size_t)index < len) {
        return vehicles[index];
    }
    return NULL;
}

int main(void) {
    const char* vehicles[] = { "Car", "Bike", "Truck", "Bus", "Scooter" };
    size_t len = sizeof(vehicles) / sizeof(vehicles[0]);

    printf("Available vehicles count: %zu\n", len);
    printf("Enter an index (0 to %zu): ", len - 1);

    char buf[64];
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
        buf[0] = '\0';
    }
    // Remove trailing newline if present
    size_t blen = strnlen(buf, sizeof(buf));
    if (blen > 0 && buf[blen - 1] == '\n') {
        buf[blen - 1] = '\0';
        blen--;
    }

    if (blen > 20) {
        printf("Invalid index.\n");
    } else {
        char* endptr = NULL;
        errno = 0;
        long val = strtol(buf, &endptr, 10);
        int valid_num = (errno == 0) && (endptr != buf) && (*endptr == '\0');
        if (valid_num) {
            const char* chosen = get_vehicle(vehicles, len, val);
            if (chosen != NULL) {
                printf("Selected vehicle: %s\n", chosen);
            } else {
                printf("Invalid index.\n");
            }
        } else {
            printf("Invalid index.\n");
        }
    }

    printf("All vehicles:\n");
    for (size_t i = 0; i < len; i++) {
        printf("%zu: %s\n", i, vehicles[i]);
    }

    printf("Test cases:\n");
    long testIndices[5] = { 0, 2, 4, -1, 10 };
    for (size_t i = 0; i < 5; i++) {
        const char* result = get_vehicle(vehicles, len, testIndices[i]);
        if (result != NULL) {
            printf("Index %ld -> %s\n", testIndices[i], result);
        } else {
            printf("Index %ld -> Invalid index.\n", testIndices[i]);
        }
    }

    return 0;
}