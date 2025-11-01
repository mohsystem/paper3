
#include <iostream>
#include <string>
#include <vector>
#include <limits>

using namespace std;

string getVehicleAtIndex(const vector<string>& vehicles, int index) {
    if (index >= 0 && index < vehicles.size()) {
        return vehicles[index];
    } else {
        return "Invalid index: Index out of bounds";
    }
}

int main() {
    vector<string> vehicles = {"Car", "Truck", "Motorcycle", "Bus", "Bicycle"};
    
    // Test case 1: Valid index 0
    cout << "Test 1 - Index 0: " << getVehicleAtIndex(vehicles, 0) << endl;
    
    // Test case 2: Valid index 2
    cout << "Test 2 - Index 2: " << getVehicleAtIndex(vehicles, 2) << endl;
    
    // Test case 3: Valid index 4
    cout << "Test 3 - Index 4: " << getVehicleAtIndex(vehicles, 4) << endl;
    
    // Test case 4: Invalid index -1
    cout << "Test 4 - Index -1: " << getVehicleAtIndex(vehicles, -1) << endl;
    
    // Test case 5: Invalid index 10
    cout << "Test 5 - Index 10: " << getVehicleAtIndex(vehicles, 10) << endl;
    
    // Interactive part
    cout << "\\nEnter an index of a vehicle (0-" << vehicles.size() - 1 << "): ";
    int index;
    if (cin >> index) {
        cout << "Vehicle: " << getVehicleAtIndex(vehicles, index) << endl;
    } else {
        cout << "Invalid input: Please enter a valid integer" << endl;
    }
    
    return 0;
}
