#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/*
Step 1: Problem understanding - vehicles array, prompt for index, return selected, print all.
Step 2: Security - validate input, use safe parsing, bounds checks, avoid buffer overflows.
Step 3 & 5: Secure coding and final secure output.
*/

const char* get_vehicle_at(const char* vehicles[], size_t len, int index) {
    if (vehicles == NULL) return NULL;
    if (index < 0) return NULL;
    if ((size_t)index >= len) return NULL;
    return vehicles[index];
}

void print_vehicles(const char* vehicles[], size_t len) {
    if (vehicles == NULL) return;
    for (size_t i = 0; i < len; ++i) {
        printf("vehicles[%zu] = %s\n", i, vehicles[i]);
    }
}

int main(void) {
    const char* vehicles[] = {"Car", "Bike", "Truck", "Bus", "Scooter"};
    size_t vehicles_len = sizeof(vehicles) / sizeof(vehicles[0]);

    printf("Enter vehicle index (0-%zu): ", vehicles_len - 1);
    char buf[64];
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
        printf("No input provided.\n");
    } else {
        char* endptr = NULL;
        errno = 0;
        long val = strtol(buf, &endptr, 10);
        if (errno != 0 || endptr == buf) {
            printf("Invalid input. Please enter an integer.\n");
        } else {
            // Skip trailing whitespace
            while (*endptr == ' ' || *endptr == '\t' || *endptr == '\n' || *endptr == '\r') endptr++;
            if (*endptr != '\0') {
                printf("Invalid input. Unexpected characters detected.\n");
            } else if (val < -2147483648L || val > 2147483647L) {
                printf("Invalid input. Integer out of range.\n");
            } else {
                int idx = (int)val;
                const char* sel = get_vehicle_at(vehicles, vehicles_len, idx);
                if (sel != NULL) {
                    printf("Selected vehicle: %s\n", sel);
                } else {
                    printf("No vehicle found at the provided index.\n");
                }
            }
        }
    }

    printf("All vehicles:\n");
    print_vehicles(vehicles, vehicles_len);

    // 5 test cases
    printf("\nTest cases:\n");
    int test_indices[5] = {0, 2, 4, -1, 10};
    for (int i = 0; i < 5; ++i) {
        const char* res = get_vehicle_at(vehicles, vehicles_len, test_indices[i]);
        printf("Test %d - index %d: %s\n", i + 1, test_indices[i], res ? res : "Invalid index");
    }

    return 0;
}