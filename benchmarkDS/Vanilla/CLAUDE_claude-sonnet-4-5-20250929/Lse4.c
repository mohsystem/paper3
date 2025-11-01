
#include <stdio.h>
#include <string.h>

const char* getVehicleAtIndex(const char* vehicles[], int size, int index) {
    if (index >= 0 && index < size) {
        return vehicles[index];
    }
    return "Invalid index";
}

int main() {
    const char* vehicles[] = {"Car", "Bike", "Truck", "Bus", "Van", "Motorcycle"};
    int size = 6;
    
    // Test case 1
    printf("Test 1: %s\\n", getVehicleAtIndex(vehicles, size, 0));
    
    // Test case 2
    printf("Test 2: %s\\n", getVehicleAtIndex(vehicles, size, 3));
    
    // Test case 3
    printf("Test 3: %s\\n", getVehicleAtIndex(vehicles, size, 5));
    
    // Test case 4
    printf("Test 4: %s\\n", getVehicleAtIndex(vehicles, size, -1));
    
    // Test case 5
    printf("Test 5: %s\\n", getVehicleAtIndex(vehicles, size, 10));
    
    // Interactive mode
    int index;
    printf("\\nEnter an index of a vehicle: ");
    scanf("%d", &index);
    printf("%s\\n", getVehicleAtIndex(vehicles, size, index));
    
    return 0;
}
