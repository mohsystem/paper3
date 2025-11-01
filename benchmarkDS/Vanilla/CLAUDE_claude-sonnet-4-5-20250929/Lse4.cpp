
#include <iostream>
#include <string>
using namespace std;

class Lse4 {
public:
    static string getVehicleAtIndex(string vehicles[], int size, int index) {
        if (index >= 0 && index < size) {
            return vehicles[index];
        }
        return "Invalid index";
    }
};

int main() {
    string vehicles[] = {"Car", "Bike", "Truck", "Bus", "Van", "Motorcycle"};
    int size = 6;
    
    // Test case 1
    cout << "Test 1: " << Lse4::getVehicleAtIndex(vehicles, size, 0) << endl;
    
    // Test case 2
    cout << "Test 2: " << Lse4::getVehicleAtIndex(vehicles, size, 3) << endl;
    
    // Test case 3
    cout << "Test 3: " << Lse4::getVehicleAtIndex(vehicles, size, 5) << endl;
    
    // Test case 4
    cout << "Test 4: " << Lse4::getVehicleAtIndex(vehicles, size, -1) << endl;
    
    // Test case 5
    cout << "Test 5: " << Lse4::getVehicleAtIndex(vehicles, size, 10) << endl;
    
    // Interactive mode
    int index;
    cout << "\\nEnter an index of a vehicle: ";
    cin >> index;
    cout << Lse4::getVehicleAtIndex(vehicles, size, index) << endl;
    
    return 0;
}
