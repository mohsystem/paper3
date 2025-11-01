#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static const char* VEHICLES[] = {"Car", "Truck", "Bike", "Bus", "Van"};
static const size_t VEHICLE_COUNT = sizeof(VEHICLES) / sizeof(VEHICLES[0]);

const char* get_vehicle(const char* input_str) {
    if (input_str == NULL) return "Invalid integer";
    char* endptr = NULL;
    // Skip leading spaces
    while (isspace((unsigned char)*input_str)) input_str++;
    long val = strtol(input_str, &endptr, 10);
    // Ensure there was a number
    if (endptr == input_str) return "Invalid integer";
    // Ensure remaining characters are only whitespace
    while (*endptr) {
        if (!isspace((unsigned char)*endptr)) return "Invalid integer";
        endptr++;
    }
    if (val >= 0 && val < (long)VEHICLE_COUNT) {
        return VEHICLES[val];
    }
    return "Invalid index";
}

int main() {
    char buffer[128];
    printf("Enter index of vehicle: ");
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        // Strip newline
        buffer[strcspn(buffer, "\r\n")] = '\0';
        printf("%s\n", get_vehicle(buffer));
    }

    // 5 test cases
    const char* tests[] = {"0", "1", "2", "3", "4"};
    for (size_t i = 0; i < 5; ++i) {
        printf("Input: %s -> %s\n", tests[i], get_vehicle(tests[i]));
    }
    return 0;
}