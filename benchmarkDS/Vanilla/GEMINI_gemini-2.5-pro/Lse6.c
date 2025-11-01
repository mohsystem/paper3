#include <stdio.h>

/**
 * @brief Returns the vehicle at a given index from the array.
 * @param vehicles The array of vehicle strings.
 * @param size The number of elements in the array.
 * @param index The index of the vehicle to retrieve.
 * @return The vehicle string or "Invalid index" if the index is out of bounds.
 */
const char* getVehicle(const char* vehicles[], int size, int index) {
    if (index >= 0 && index < size) {
        return vehicles[index];
    } else {
        return "Invalid index";
    }
}

/**
 * @brief Loops through the array and prints each vehicle.
 * @param vehicles The array of vehicle strings.
 * @param size The number of elements in the array.
 */
void printAllVehicles(const char* vehicles[], int size) {
    printf("All vehicles:\n");
    for (int i = 0; i < size; i++) {
        printf("%s\n", vehicles[i]);
    }
    printf("\n");
}

int main() {
    // Create an array of strings called vehicles
    const char* vehicles[] = {"Car", "Bike", "Bus", "Train", "Plane"};
    int n = sizeof(vehicles) / sizeof(vehicles[0]);

    // Loop through the array and print out each vehicle
    printAllVehicles(vehicles, n);

    // 5 test cases for getting a vehicle by index
    printf("--- Test Cases for getVehicle ---\n");
    
    // Test Case 1
    int index1 = 0;
    printf("Vehicle at index %d: %s\n", index1, getVehicle(vehicles, n, index1));
    
    // Test Case 2
    int index2 = 2;
    printf("Vehicle at index %d: %s\n", index2, getVehicle(vehicles, n, index2));
    
    // Test Case 3
    int index3 = 4;
    printf("Vehicle at index %d: %s\n", index3, getVehicle(vehicles, n, index3));
    
    // Test Case 4 (out of bounds - upper)
    int index4 = 5;
    printf("Vehicle at index %d: %s\n", index4, getVehicle(vehicles, n, index4));

    // Test Case 5 (out of bounds - lower)
    int index5 = -1;
    printf("Vehicle at index %d: %s\n", index5, getVehicle(vehicles, n, index5));

    return 0;
}