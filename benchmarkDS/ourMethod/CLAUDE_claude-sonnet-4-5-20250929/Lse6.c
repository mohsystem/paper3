
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

// Define maximum array size to prevent excessive memory usage
#define MAX_ARRAY_SIZE 100
#define MAX_INPUT_LENGTH 256
#define MAX_VEHICLE_NAME_LENGTH 100

// Returns vehicle at specified index with input validation
// Returns NULL on error, caller must check return value
const char* get_vehicle_by_index(const char* vehicles[], int num_vehicles, int index) {
    // Validate input: check if vehicles array is NULL
    if (vehicles == NULL) {
        return NULL;
    }
    
    // Validate input: check if index is within valid range
    // This prevents out-of-bounds access and potential buffer overflows
    if (index < 0 || index >= num_vehicles) {
        return NULL;
    }
    
    // Return the vehicle at the specified index
    return vehicles[index];
}

// Prints all vehicles in the array with validation
void print_all_vehicles(const char* vehicles[], int num_vehicles) {
    // Validate input: check if vehicles array is NULL
    if (vehicles == NULL) {
        fprintf(stderr, "Error: Invalid vehicle array\\n");
        return;
    }
    
    // Loop through array and print each vehicle
    printf("All vehicles:\\n");
    for (int i = 0; i < num_vehicles; i++) {
        // Validate each element is not NULL before printing
        if (vehicles[i] != NULL) {
            printf("%s\\n", vehicles[i]);
        }
    }
}

// Safe integer parsing with validation
// Returns 1 on success, 0 on failure
int safe_parse_int(const char* str, int* result) {
    // Validate input pointer
    if (str == NULL || result == NULL) {
        return 0;
    }
    
    // Clear errno before conversion
    errno = 0;
    char* endptr = NULL;
    
    // Use strtol for safe conversion
    long value = strtol(str, &endptr, 10);
    
    // Check for conversion errors
    if (errno != 0 || endptr == str || *endptr != '\\n' && *endptr != '\\0') {
        return 0;
    }
    
    // Check for integer overflow
    if (value > INT_MAX || value < INT_MIN) {
        return 0;
    }
    
    *result = (int)value;
    return 1;
}

int main(void) {
    // Create array of vehicle strings (const to prevent modification)
    const char* vehicles[] = {"Car", "Truck", "Motorcycle", "Bus", "Bicycle"};
    int num_vehicles = sizeof(vehicles) / sizeof(vehicles[0]);
    
    // Test cases
    printf("=== Test Case 1: Valid index 0 ===\\n");
    const char* result1 = get_vehicle_by_index(vehicles, num_vehicles, 0);
    if (result1 != NULL) {
        printf("%s\\n", result1);
    } else {
        printf("Error: Invalid index\\n");
    }
    print_all_vehicles(vehicles, num_vehicles);
    
    printf("\\n=== Test Case 2: Valid index 2 ===\\n");
    const char* result2 = get_vehicle_by_index(vehicles, num_vehicles, 2);
    if (result2 != NULL) {
        printf("%s\\n", result2);
    } else {
        printf("Error: Invalid index\\n");
    }
    print_all_vehicles(vehicles, num_vehicles);
    
    printf("\\n=== Test Case 3: Invalid negative index ===\\n");
    const char* result3 = get_vehicle_by_index(vehicles, num_vehicles, -1);
    if (result3 != NULL) {
        printf("%s\\n", result3);
    } else {
        printf("Error: Invalid index\\n");
    }
    
    printf("\\n=== Test Case 4: Invalid index too large ===\\n");
    const char* result4 = get_vehicle_by_index(vehicles, num_vehicles, 10);
    if (result4 != NULL) {
        printf("%s\\n", result4);
    } else {
        printf("Error: Invalid index\\n");
    }
    
    printf("\\n=== Test Case 5: Valid index at boundary ===\\n");
    const char* result5 = get_vehicle_by_index(vehicles, num_vehicles, 4);
    if (result5 != NULL) {
        printf("%s\\n", result5);
    } else {
        printf("Error: Invalid index\\n");
    }
    
    // Interactive prompt with input validation
    printf("\\n=== Interactive Mode ===\\n");
    printf("Enter vehicle index (0-%d): ", num_vehicles - 1);
    
    // Allocate buffer for input with fixed size to prevent overflow
    char input_buffer[MAX_INPUT_LENGTH];
    // Initialize buffer to prevent use of uninitialized memory
    memset(input_buffer, 0, sizeof(input_buffer));
    
    // Use fgets for safe input reading with length limit
    // fgets is safer than scanf as it prevents buffer overflow
    if (fgets(input_buffer, sizeof(input_buffer), stdin) != NULL) {
        int user_index;
        
        // Validate and parse input using safe integer parsing
        if (safe_parse_int(input_buffer, &user_index)) {
            const char* result = get_vehicle_by_index(vehicles, num_vehicles, user_index);
            if (result != NULL) {
                printf("Result: %s\\n", result);
            } else {
                // Generic error message to avoid leaking internal details
                printf("Error: Invalid index\\n");
            }
        } else {
            // Generic error message to avoid leaking internal details
            printf("Error: Invalid input format\\n");
        }
    } else {
        // Generic error message to avoid leaking internal details
        printf("Error: Unable to read input\\n");
    }
    
    return 0;
}
