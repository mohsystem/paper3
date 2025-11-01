
#include <stdio.h>
#include <string.h>

const char* getVehicle(const char* vehicles[], int size, int index) {
    if (index >= 0 && index < size) {
        return vehicles[index];
    }
    return "Invalid index";
}

void printAllVehicles(const char* vehicles[], int size) {
    for (int i = 0; i < size; i++) {
        printf("%s\\n", vehicles[i]);
    }
}

int main() {
    const char* vehicles[] = {"Car", "Truck", "Motorcycle", "Bus", "Bicycle"};
    int size = 5;
    
    // Test case 1: Valid index 0
    printf("Test 1 - Index 0: %s\\n", getVehicle(vehicles, size, 0));
    
    // Test case 2: Valid index 2
    printf("Test 2 - Index 2: %s\\n", getVehicle(vehicles, size, 2));
    
    // Test case 3: Valid index 4
    printf("Test 3 - Index 4: %s\\n", getVehicle(vehicles, size, 4));
    
    // Test case 4: Invalid index -1
    printf("Test 4 - Index -1: %s\\n", getVehicle(vehicles, size, -1));
    
    // Test case 5: Invalid index 10
    printf("Test 5 - Index 10: %s\\n", getVehicle(vehicles, size, 10));
    
    printf("\\nAll vehicles:\\n");
    printAllVehicles(vehicles, size);
    
    return 0;
}
