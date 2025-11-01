#include <stdio.h>

const char* VEHICLES[] = {"Car", "Bike", "Bus", "Truck", "Van"};
// Calculate the number of elements in the array at compile time
const int NUM_VEHICLES = sizeof(VEHICLES) / sizeof(VEHICLES[0]);

/**
 * @brief Gets a vehicle from the predefined list by its index.
 * 
 * @param index The index of the vehicle to retrieve.
 * @return A const char pointer to the vehicle name, or "Invalid index" if the index is out of bounds.
 */
const char* getVehicleByIndex(int index) {
    // Security Check: Validate that the index is within the valid range.
    if (index < 0 || index >= NUM_VEHICLES) {
        return "Invalid index";
    }
    return VEHICLES[index];
}

int main() {
    // 5 Test Cases
    int testIndexes[] = {0, 2, 4, -1, 5};
    int numTests = sizeof(testIndexes) / sizeof(testIndexes[0]);

    printf("Running C Test Cases:\n");
    for (int i = 0; i < numTests; ++i) {
        int index = testIndexes[i];
        const char* vehicle = getVehicleByIndex(index);
        printf("Index: %d -> Vehicle: %s\n", index, vehicle);
    }

    return 0;
}