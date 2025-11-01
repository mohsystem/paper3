
#include <stdio.h>
#include <string.h>

const char* getVehicleAtIndex(const char* vehicles[], int vehiclesSize, int index, char* errorMsg) {
    if (index >= 0 && index < vehiclesSize) {
        return vehicles[index];
    } else {
        strcpy(errorMsg, "Invalid index: Index out of bounds");
        return errorMsg;
    }
}

int main() {
    const char* vehicles[] = {"Car", "Truck", "Motorcycle", "Bus", "Bicycle"};
    int vehiclesSize = 5;
    char errorMsg[100];
    
    // Test case 1: Valid index 0
    printf("Test 1 - Index 0: %s\\n", getVehicleAtIndex(vehicles, vehiclesSize, 0, errorMsg));
    
    // Test case 2: Valid index 2
    printf("Test 2 - Index 2: %s\\n", getVehicleAtIndex(vehicles, vehiclesSize, 2, errorMsg));
    
    // Test case 3: Valid index 4
    printf("Test 3 - Index 4: %s\\n", getVehicleAtIndex(vehicles, vehiclesSize, 4, errorMsg));
    
    // Test case 4: Invalid index -1
    printf("Test 4 - Index -1: %s\\n", getVehicleAtIndex(vehicles, vehiclesSize, -1, errorMsg));
    
    // Test case 5: Invalid index 10
    printf("Test 5 - Index 10: %s\\n", getVehicleAtIndex(vehicles, vehiclesSize, 10, errorMsg));
    
    // Interactive part
    printf("\\nEnter an index of a vehicle (0-%d): ", vehiclesSize - 1);
    int index;
    if (scanf("%d", &index) == 1) {
        printf("Vehicle: %s\\n", getVehicleAtIndex(vehicles, vehiclesSize, index, errorMsg));
    } else {
        printf("Invalid input: Please enter a valid integer\\n");
    }
    
    return 0;
}
