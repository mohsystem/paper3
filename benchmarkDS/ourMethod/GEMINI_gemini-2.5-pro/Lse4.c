#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

static const char* VEHICLES[] = {"Car", "Bus", "Motorcycle", "Airplane", "Boat"};
static const size_t NUM_VEHICLES = sizeof(VEHICLES) / sizeof(VEHICLES[0]);

/**
 * @brief Retrieves a vehicle from the array by its index and copies it to a buffer.
 * 
 * @param index The index of the vehicle to retrieve.
 * @param result_buffer The buffer to store the output string.
 * @param buffer_size The size of the result_buffer.
 */
void getVehicle(long index, char* result_buffer, size_t buffer_size) {
    // Rule #1, #4: Ensure array access is within boundaries.
    if (index >= 0 && (size_t)index < NUM_VEHICLES) {
        snprintf(result_buffer, buffer_size, "%s", VEHICLES[index]);
    } else {
        snprintf(result_buffer, buffer_size, "Error: Index is out of bounds.");
    }
}

/**
 * @brief Processes a string input, converts it to an integer, and gets the vehicle.
 * 
 * @param input The string input representing the index.
 * @param result_buffer The buffer to store the output string.
 * @param buffer_size The size of the result_buffer.
 */
void processInput(const char* input, char* result_buffer, size_t buffer_size) {
    char* endptr;
    errno = 0; // Reset errno before the call

    // Rule #4, #5: Use strtol for safe string-to-long conversion.
    long index = strtol(input, &endptr, 10);

    // Check for various conversion errors.
    if (errno == ERANGE) {
        snprintf(result_buffer, buffer_size, "Error: Input number is out of range.");
        return;
    }

    // Check if any digits were read.
    if (endptr == input) {
        snprintf(result_buffer, buffer_size, "Error: Input is not a valid integer.");
        return;
    }

    // Check if there are any non-whitespace characters left in the string.
    while (*endptr != '\0') {
        if (!(*endptr == ' ' || *endptr == '\t' || *endptr == '\n' || *endptr == '\r')) {
            snprintf(result_buffer, buffer_size, "Error: Input contains non-numeric characters.");
            return;
        }
        endptr++;
    }
    
    // Check if the parsed long value fits into an int if needed, though for an index it's fine.
    if (index > INT_MAX || index < INT_MIN) {
         snprintf(result_buffer, buffer_size, "Error: Input number is out of integer range.");
         return;
    }

    getVehicle(index, result_buffer, buffer_size);
}


int main() {
    // In a real application, you would get user input:
    // char input_buffer[256];
    // printf("Enter an index of a vehicle: ");
    // if (fgets(input_buffer, sizeof(input_buffer), stdin)) {
    //     char result_buffer[256];
    //     processInput(input_buffer, result_buffer, sizeof(result_buffer));
    //     printf("Vehicle: %s\n", result_buffer);
    // }

    // For demonstration, here are 5 hardcoded test cases.
    const char* testInputs[] = {"2", "0", "4", "10", "-1", "Airplane"};
    size_t num_tests = sizeof(testInputs) / sizeof(testInputs[0]);

    printf("Running test cases...\n");
    for (size_t i = 0; i < num_tests; ++i) {
        char result[256];
        printf("\nTest Case %zu:\n", i + 1);
        printf("Input: \"%s\"\n", testInputs[i]);
        processInput(testInputs[i], result, sizeof(result));
        printf("Output: %s\n", result);
    }

    return 0;
}