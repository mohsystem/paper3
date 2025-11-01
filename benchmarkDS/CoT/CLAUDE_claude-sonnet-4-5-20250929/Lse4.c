
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#define VEHICLE_COUNT 5
#define MAX_INPUT_LENGTH 100

const char* vehicles[VEHICLE_COUNT] = {
    "Toyota Camry",
    "Honda Civic",
    "Ford F-150",
    "Tesla Model 3",
    "Chevrolet Silverado"
};

const char* getVehicleAtIndex(int index) {
    // Validate index bounds
    if (index < 0 || index >= VEHICLE_COUNT) {
        return "Error: Index out of bounds";
    }
    return vehicles[index];
}

void runTests() {
    printf("=== Test Cases ===\\n");
    printf("Test 1 (index 0): %s\\n", getVehicleAtIndex(0));
    printf("Test 2 (index 2): %s\\n", getVehicleAtIndex(2));
    printf("Test 3 (index 4): %s\\n", getVehicleAtIndex(4));
    printf("Test 4 (index -1): %s\\n", getVehicleAtIndex(-1));
    printf("Test 5 (index 10): %s\\n", getVehicleAtIndex(10));
}

void runInteractive() {
    char input[MAX_INPUT_LENGTH];
    char *endptr;
    long index;
    
    printf("\\n=== Interactive Mode ===\\n");
    printf("Enter an index of a vehicle (0-%d): ", VEHICLE_COUNT - 1);
    
    if (fgets(input, MAX_INPUT_LENGTH, stdin) == NULL) {
        printf("Error: Failed to read input\\n");
        return;
    }
    
    // Remove newline character
    input[strcspn(input, "\\n")] = 0;
    
    // Validate input is not empty
    if (strlen(input) == 0) {
        printf("Error: Input cannot be empty\\n");
        return;
    }
    
    // Parse and validate integer
    errno = 0;
    index = strtol(input, &endptr, 10);
    
    // Check for conversion errors
    if (errno == ERANGE || *endptr != '\\0' || endptr == input) {
        printf("Error: Invalid input. Please enter a valid integer.\\n");
        return;
    }
    
    // Check for overflow to int
    if (index < INT_MIN || index > INT_MAX) {
        printf("Error: Number out of valid range\\n");
        return;
    }
    
    printf("Result: %s\\n", getVehicleAtIndex((int)index));
}

int main() {
    runTests();
    runInteractive();
    
    return 0;
}
