#include <stdio.h>

/**
 * @brief Retrieves a vehicle from the array at a specified index.
 * 
 * Includes bounds checking to prevent out-of-bounds access.
 * 
 * @param vehicles The array of vehicle strings (char pointers).
 * @param size The total number of elements in the array.
 * @param index The index of the vehicle to retrieve.
 * @return The vehicle string at the given index, or an error message if invalid.
 */
const char* getVehicle(const char* vehicles[], int size, int index) {
    if (index >= 0 && index < size) {
        return vehicles[index];
    } else {
        // This static buffer is not thread-safe, but simple for this example.
        // For a more robust solution, the caller would provide a buffer.
        static char errorMsg[50];
        sprintf(errorMsg, "Error: Index %d is out of bounds.", index);
        return errorMsg;
    }
}

/**
 * @brief Loops through the array and prints each vehicle.
 * 
 * @param vehicles The array of vehicle strings (char pointers).
 * @param size The total number of elements in the array.
 */
void printAllVehicles(const char* vehicles[], int size) {
    printf("\n--- All Vehicles ---\n");
    for (int i = 0; i < size; i++) {
        printf("%s\n", vehicles[i]);
    }
    printf("--------------------\n");
}

int main() {
    const char* vehicles[] = {"Car", "Bike", "Bus", "Train", "Plane"};
    int num_vehicles = sizeof(vehicles) / sizeof(vehicles[0]);

    // Print all vehicles
    printAllVehicles(vehicles, num_vehicles);

    // 5 Test Cases
    printf("\n--- Test Cases ---\n");
    int testIndices[] = {1, 3, 0, -1, 5};
    int num_tests = sizeof(testIndices) / sizeof(testIndices[0]);

    for (int i = 0; i < num_tests; i++) {
        int index = testIndices[i];
        printf("Test Case %d: Getting vehicle at index %d\n", (i + 1), index);
        const char* vehicle = getVehicle(vehicles, num_vehicles, index);
        printf("Result: %s\n", vehicle);
    }

    return 0;
}