
#include <iostream>
#include <string>
using namespace std;

string getVehicle(string vehicles[], int size, int index) {
    if (index >= 0 && index < size) {
        return vehicles[index];
    }
    return "Invalid index";
}

void printAllVehicles(string vehicles[], int size) {
    for (int i = 0; i < size; i++) {
        cout << vehicles[i] << endl;
    }
}

int main() {
    string vehicles[] = {"Car", "Truck", "Motorcycle", "Bus", "Bicycle"};
    int size = 5;
    
    // Test case 1: Valid index 0
    cout << "Test 1 - Index 0: " << getVehicle(vehicles, size, 0) << endl;
    
    // Test case 2: Valid index 2
    cout << "Test 2 - Index 2: " << getVehicle(vehicles, size, 2) << endl;
    
    // Test case 3: Valid index 4
    cout << "Test 3 - Index 4: " << getVehicle(vehicles, size, 4) << endl;
    
    // Test case 4: Invalid index -1
    cout << "Test 4 - Index -1: " << getVehicle(vehicles, size, -1) << endl;
    
    // Test case 5: Invalid index 10
    cout << "Test 5 - Index 10: " << getVehicle(vehicles, size, 10) << endl;
    
    cout << "\\nAll vehicles:" << endl;
    printAllVehicles(vehicles, size);
    
    return 0;
}
