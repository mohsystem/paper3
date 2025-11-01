#include <stdio.h>
#include <string.h>

/**
 * @brief Securely retrieves a vehicle from the array by index.
 * 
 * @param vehicles The array of vehicles.
 * @param size The number of elements in the array.
 * @param index The index to retrieve.
 * @return The vehicle at the specified index or an error message if the index is invalid.
 */
const char* getVehicleAtIndex(const char* vehicles[], int size, int index) {
    // Security check: Ensure the index is within the valid range.
    if (index >= 0 && index < size) {
        return vehicles[index];
    } else {
        return "Error: Invalid index provided.";
    }
}

/**
 * @brief Loops through the array and prints each vehicle.
 * 
 * @param vehicles The array of vehicles to print.
 * @param size The number of elements in the array.
 */
void printAllVehicles(const char* vehicles[], int size) {
    printf("\n--- All Vehicles ---\n");
    for (int i = 0; i < size; ++i) {
        printf("%s\n", vehicles[i]);
    }
    printf("--------------------\n");
}

int main() {
    const char* vehicles[] = {"Car", "Bike", "Bus", "Train", "Plane"};
    int size = sizeof(vehicles) / sizeof(vehicles[0]);

    // --- 5 Test Cases ---
    printf("--- Test Cases ---\n");
    
    // Test Case 1: Valid index (first element)
    int index1 = 0;
    printf("Requesting vehicle at index %d: %s\n", index1, getVehicleAtIndex(vehicles, size, index1));

    // Test Case 2: Valid index (middle element)
    int index2 = 2;
    printf("Requesting vehicle at index %d: %s\n", index2, getVehicleAtIndex(vehicles, size, index2));

    // Test Case 3: Valid index (last element)
    int index3 = 4;
    printf("Requesting vehicle at index %d: %s\n", index3, getVehicleAtIndex(vehicles, size, index3));

    // Test Case 4: Invalid index (out of bounds - too high)
    int index4 = 10;
    printf("Requesting vehicle at index %d: %s\n", index4, getVehicleAtIndex(vehicles, size, index4));
    
    // Test Case 5: Invalid index (out of bounds - negative)
    int index5 = -1;
    printf("Requesting vehicle at index %d: %s\n", index5, getVehicleAtIndex(vehicles, size, index5));
    
    printf("------------------\n");

    // Loop through and print all vehicles
    printAllVehicles(vehicles, size);

    return 0;
}