#include <stdio.h>

/**
 * Accesses an array of vehicles at a given index.
 *
 * @param vehicles An array of vehicle names (strings).
 * @param size The number of elements in the array.
 * @param index The index to access.
 * @return The vehicle name at the specified index, or an error message if the index is out of bounds.
 */
const char* getVehicleByIndex(const char* vehicles[], int size, int index) {
    if (index >= 0 && index < size) {
        return vehicles[index];
    } else {
        return "Error: Index is out of bounds.";
    }
}

int main() {
    const char* vehicles[] = {"car", "bus", "bike", "train", "plane"};
    int size = sizeof(vehicles) / sizeof(vehicles[0]);

    // Test cases
    printf("Index 0: %s\n", getVehicleByIndex(vehicles, size, 0));  // Expected: car
    printf("Index 2: %s\n", getVehicleByIndex(vehicles, size, 2));  // Expected: bike
    printf("Index 4: %s\n", getVehicleByIndex(vehicles, size, 4));  // Expected: plane
    printf("Index -1: %s\n", getVehicleByIndex(vehicles, size, -1)); // Expected: Error
    printf("Index 5: %s\n", getVehicleByIndex(vehicles, size, 5));  // Expected: Error

    return 0;
}