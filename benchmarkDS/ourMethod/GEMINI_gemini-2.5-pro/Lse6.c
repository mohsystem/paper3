#include <stdio.h>
#include <stddef.h> // For size_t

/**
 * @brief Retrieves a vehicle from the array at the specified index.
 *
 * @param vehicles The array of vehicle strings.
 * @param count The number of elements in the vehicles array.
 * @param index The index of the vehicle to retrieve.
 * @return The vehicle string at the given index, or an error message if the index is invalid.
 */
const char* getVehicle(const char* vehicles[], size_t count, size_t index) {
    // Rule #1, #3, #4: Input validation - check if the index is within the valid bounds.
    if (vehicles != NULL && index < count) {
        return vehicles[index];
    } else {
        return "Error: Index out of bounds.";
    }
}

/**
 * @brief Prints all vehicles in the given array to the console.
 *
 * @param vehicles The array of vehicle strings.
 * @param count The number of elements in the vehicles array.
 */
void printAllVehicles(const char* vehicles[], size_t count) {
    if (vehicles == NULL) return;
    printf("All vehicles:\n");
    for (size_t i = 0; i < count; ++i) {
        // Defensive check for NULL pointers within the array
        if (vehicles[i] != NULL) {
            printf("%s\n", vehicles[i]);
        }
    }
}

int main(void) {
    // The array of strings called vehicles.
    const char* vehicles[] = {"Car", "Bus", "Motorcycle", "Bicycle", "Truck"};
    // Calculate the number of elements safely.
    size_t num_vehicles = sizeof(vehicles) / sizeof(vehicles[0]);

    // Loop through the array and print out each vehicle.
    printAllVehicles(vehicles, num_vehicles);
    printf("--------------------\n");

    // 5 test cases
    printf("Running test cases...\n");

    // Test Case 1: Valid index (0)
    size_t testIndex1 = 0;
    printf("Getting vehicle at index %zu: %s\n", testIndex1, getVehicle(vehicles, num_vehicles, testIndex1));

    // Test Case 2: Valid index (middle)
    size_t testIndex2 = 2;
    printf("Getting vehicle at index %zu: %s\n", testIndex2, getVehicle(vehicles, num_vehicles, testIndex2));

    // Test Case 3: Valid index (last element)
    size_t testIndex3 = 4;
    printf("Getting vehicle at index %zu: %s\n", testIndex3, getVehicle(vehicles, num_vehicles, testIndex3));

    // Test Case 4: Invalid index (out of bounds)
    size_t testIndex4 = 5;
    printf("Getting vehicle at index %zu: %s\n", testIndex4, getVehicle(vehicles, num_vehicles, testIndex4));

    // Test Case 5: Invalid index (large number)
    size_t testIndex5 = 100;
    printf("Getting vehicle at index %zu: %s\n", testIndex5, getVehicle(vehicles, num_vehicles, testIndex5));

    return 0;
}