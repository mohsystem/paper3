
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

// Predefined array of vehicles with size constant
#define VEHICLES_SIZE 5
static const char* VEHICLES[VEHICLES_SIZE] = {"Car", "Truck", "Motorcycle", "Bus", "Van"};

// Function to safely get vehicle at index with bounds checking
// Returns 0 on success, -1 on error
int getVehicleAtIndex(int index, char* output, size_t output_size) {
    // Initialize output pointer for safety
    if (output == NULL || output_size == 0) {
        return -1;
    }
    
    // Input validation: check bounds to prevent buffer overflow
    // This prevents out of bounds access vulnerabilities
    if (index < 0 || index >= VEHICLES_SIZE) {
        // Use snprintf for bounds-safe string operations
        snprintf(output, output_size, "Error: Invalid index");
        return -1;
    }
    
    // Safe string copy with bounds checking
    // Ensure null termination
    size_t vehicle_len = strlen(VEHICLES[index]);
    if (vehicle_len >= output_size) {
        snprintf(output, output_size, "Error: Buffer too small");
        return -1;
    }
    
    // Use memcpy with explicit size for safe copy
    memcpy(output, VEHICLES[index], vehicle_len);
    output[vehicle_len] = '\\0'; // Ensure null termination
    
    return 0;
}

int main(void) {
    char input_buffer[32]; // Buffer for user input with size limit
    char output_buffer[256]; // Buffer for output
    int index;
    char* endptr = NULL;
    long parsed_value;
    
    // Initialize buffers to zero for safety
    memset(input_buffer, 0, sizeof(input_buffer));
    memset(output_buffer, 0, sizeof(output_buffer));
    
    // Prompt user for input
    printf("Enter an index of a vehicle (0-%d): ", VEHICLES_SIZE - 1);
    fflush(stdout);
    
    // Use fgets for bounds-safe input reading
    // This prevents buffer overflow vulnerabilities
    if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) {
        printf("Error: Failed to read input\\n");
        return 1;
    }
    
    // Remove newline if present
    size_t len = strlen(input_buffer);
    if (len > 0 && input_buffer[len - 1] == '\\n') {
        input_buffer[len - 1] = '\\0';
    }
    
    // Input validation: convert string to integer safely
    // Use strtol with error checking to prevent undefined behavior
    errno = 0;
    parsed_value = strtol(input_buffer, &endptr, 10);
    
    // Check for conversion errors
    if (errno != 0 || endptr == input_buffer || *endptr != '\\0') {
        printf("Error: Invalid input format\\n");
        return 1;
    }
    
    // Check for integer overflow/underflow
    if (parsed_value > INT_MAX || parsed_value < INT_MIN) {
        printf("Error: Input out of range\\n");
        return 1;
    }
    
    index = (int)parsed_value;
    
    // Get vehicle using validated index
    if (getVehicleAtIndex(index, output_buffer, sizeof(output_buffer)) == 0) {
        printf("%s\\n", output_buffer);
    } else {
        printf("%s\\n", output_buffer);
    }
    
    // Test cases
    printf("\\n--- Test Cases ---\\n");
    
    getVehicleAtIndex(0, output_buffer, sizeof(output_buffer));
    printf("Test 1 (index 0): %s\\n", output_buffer);
    
    getVehicleAtIndex(2, output_buffer, sizeof(output_buffer));
    printf("Test 2 (index 2): %s\\n", output_buffer);
    
    getVehicleAtIndex(4, output_buffer, sizeof(output_buffer));
    printf("Test 3 (index 4): %s\\n", output_buffer);
    
    getVehicleAtIndex(-1, output_buffer, sizeof(output_buffer));
    printf("Test 4 (index -1): %s\\n", output_buffer);
    
    getVehicleAtIndex(10, output_buffer, sizeof(output_buffer));
    printf("Test 5 (index 10): %s\\n", output_buffer);
    
    return 0;
}
