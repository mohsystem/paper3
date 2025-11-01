#include <stdio.h>

/**
 * Securely retrieves a vehicle from an array by index.
 *
 * @param vehicles An array of C-style strings (char pointers).
 * @param num_vehicles The number of elements in the vehicles array.
 * @param index The index of the vehicle to retrieve.
 * @return The vehicle name at the given index or an error message if the index is invalid.
 */
const char* getVehicle(const char* vehicles[], int num_vehicles, int index) {
    // Security check: Validate that the index is within the legitimate bounds of the array.
    // This prevents out-of-bounds memory access, which is undefined behavior in C.
    if (index >= 0 && index < num_vehicles) {
        return vehicles[index];
    } else {
        return "Error: Index is out of bounds.";
    }
}

int main() {
    const char* vehicleArray[] = {"Car", "Motorcycle", "Truck", "Bus", "Bicycle"};
    int num_vehicles = sizeof(vehicleArray) / sizeof(vehicleArray[0]);

    printf("--- 5 Test Cases ---\n");

    int testIndices[] = {
        0,  // Test Case 1: Valid index (lower bound)
        2,  // Test Case 2: Valid index (middle)
        4,  // Test Case 3: Valid index (upper bound)
        -1, // Test Case 4: Invalid index (negative)
        5   // Test Case 5: Invalid index (too high)
    };
    int num_tests = sizeof(testIndices) / sizeof(testIndices[0]);

    for (int i = 0; i < num_tests; ++i) {
        int index = testIndices[i];
        const char* result = getVehicle(vehicleArray, num_vehicles, index);
        printf("Requesting vehicle at index %d: %s\n", index, result);
    }

    return 0;
}