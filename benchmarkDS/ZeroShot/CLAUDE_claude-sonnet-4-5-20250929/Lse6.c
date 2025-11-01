
#include <stdio.h>
#include <string.h>

const char* getVehicle(char vehicles[][20], int size, int index) {
    static char invalid[] = "Invalid index";
    if (index >= 0 && index < size) {
        return vehicles[index];
    }
    return invalid;
}

void printAllVehicles(char vehicles[][20], int size) {
    for (int i = 0; i < size; i++) {
        printf("%s\\n", vehicles[i]);
    }
}

int main() {
    char vehicles[][20] = {"Car", "Truck", "Motorcycle", "Bus", "Bicycle"};
    int size = 5;
    
    // Test cases
    printf("Test Case 1: Index 0\\n");
    printf("Vehicle: %s\\n", getVehicle(vehicles, size, 0));
    printf("\\n");
    
    printf("Test Case 2: Index 2\\n");
    printf("Vehicle: %s\\n", getVehicle(vehicles, size, 2));
    printf("\\n");
    
    printf("Test Case 3: Index 4\\n");
    printf("Vehicle: %s\\n", getVehicle(vehicles, size, 4));
    printf("\\n");
    
    printf("Test Case 4: Invalid index -1\\n");
    printf("Vehicle: %s\\n", getVehicle(vehicles, size, -1));
    printf("\\n");
    
    printf("Test Case 5: Invalid index 10\\n");
    printf("Vehicle: %s\\n", getVehicle(vehicles, size, 10));
    printf("\\n");
    
    printf("All vehicles:\\n");
    printAllVehicles(vehicles, size);
    
    return 0;
}
