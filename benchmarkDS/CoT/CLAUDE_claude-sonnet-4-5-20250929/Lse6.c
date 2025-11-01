
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_VEHICLES 10
#define MAX_STRING_LENGTH 50

const char* getVehicleAtIndex(char vehicles[][MAX_STRING_LENGTH], int vehicleCount, int index, char* result) {
    // Input validation and bounds checking
    if (vehicles == NULL || vehicleCount <= 0) {
        strcpy(result, "Error: Vehicle array is empty or NULL");
        return result;
    }
    
    if (index < 0 || index >= vehicleCount) {
        snprintf(result, MAX_STRING_LENGTH, "Error: Index out of bounds. Valid range: 0-%d", vehicleCount - 1);
        return result;
    }
    
    strcpy(result, vehicles[index]);
    return result;
}

void printAllVehicles(char vehicles[][MAX_STRING_LENGTH], int vehicleCount) {
    if (vehicles == NULL || vehicleCount <= 0) {
        printf("No vehicles to display.\\n");
        return;
    }
    
    printf("\\nAll vehicles:\\n");
    for (int i = 0; i < vehicleCount; i++) {
        printf("Index %d: %s\\n", i, vehicles[i]);
    }
}

int main() {
    char vehicles[MAX_VEHICLES][MAX_STRING_LENGTH] = {
        "Car", "Truck", "Motorcycle", "Bus", "Bicycle"
    };
    int vehicleCount = 5;
    char result[MAX_STRING_LENGTH];
    
    // Test cases
    printf("=== Test Case 1: Valid index 0 ===\\n");
    printf("Vehicle at index 0: %s\\n", getVehicleAtIndex(vehicles, vehicleCount, 0, result));
    printAllVehicles(vehicles, vehicleCount);
    
    printf("\\n=== Test Case 2: Valid index 3 ===\\n");
    printf("Vehicle at index 3: %s\\n", getVehicleAtIndex(vehicles, vehicleCount, 3, result));
    printAllVehicles(vehicles, vehicleCount);
    
    printf("\\n=== Test Case 3: Invalid negative index ===\\n");
    printf("Vehicle at index -1: %s\\n", getVehicleAtIndex(vehicles, vehicleCount, -1, result));
    printAllVehicles(vehicles, vehicleCount);
    
    printf("\\n=== Test Case 4: Invalid index too large ===\\n");
    printf("Vehicle at index 10: %s\\n", getVehicleAtIndex(vehicles, vehicleCount, 10, result));
    printAllVehicles(vehicles, vehicleCount);
    
    printf("\\n=== Test Case 5: Last valid index ===\\n");
    printf("Vehicle at index 4: %s\\n", getVehicleAtIndex(vehicles, vehicleCount, 4, result));
    printAllVehicles(vehicles, vehicleCount);
    
    return 0;
}
