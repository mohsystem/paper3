
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Lse6 {
public:
    static string getVehicleAtIndex(const vector<string>& vehicles, int index) {
        // Input validation and bounds checking
        if (vehicles.empty()) {
            return "Error: Vehicle array is empty";
        }
        
        if (index < 0 || index >= static_cast<int>(vehicles.size())) {
            return "Error: Index out of bounds. Valid range: 0-" + 
                   to_string(vehicles.size() - 1);
        }
        
        return vehicles[index];
    }
    
    static void printAllVehicles(const vector<string>& vehicles) {
        if (vehicles.empty()) {
            cout << "No vehicles to display." << endl;
            return;
        }
        
        cout << "\\nAll vehicles:" << endl;
        for (size_t i = 0; i < vehicles.size(); i++) {
            cout << "Index " << i << ": " << vehicles[i] << endl;
        }
    }
};

int main() {
    vector<string> vehicles = {"Car", "Truck", "Motorcycle", "Bus", "Bicycle"};
    
    // Test cases
    cout << "=== Test Case 1: Valid index 0 ===" << endl;
    cout << "Vehicle at index 0: " << Lse6::getVehicleAtIndex(vehicles, 0) << endl;
    Lse6::printAllVehicles(vehicles);
    
    cout << "\\n=== Test Case 2: Valid index 3 ===" << endl;
    cout << "Vehicle at index 3: " << Lse6::getVehicleAtIndex(vehicles, 3) << endl;
    Lse6::printAllVehicles(vehicles);
    
    cout << "\\n=== Test Case 3: Invalid negative index ===" << endl;
    cout << "Vehicle at index -1: " << Lse6::getVehicleAtIndex(vehicles, -1) << endl;
    Lse6::printAllVehicles(vehicles);
    
    cout << "\\n=== Test Case 4: Invalid index too large ===" << endl;
    cout << "Vehicle at index 10: " << Lse6::getVehicleAtIndex(vehicles, 10) << endl;
    Lse6::printAllVehicles(vehicles);
    
    cout << "\\n=== Test Case 5: Last valid index ===" << endl;
    cout << "Vehicle at index 4: " << Lse6::getVehicleAtIndex(vehicles, 4) << endl;
    Lse6::printAllVehicles(vehicles);
    
    return 0;
}
