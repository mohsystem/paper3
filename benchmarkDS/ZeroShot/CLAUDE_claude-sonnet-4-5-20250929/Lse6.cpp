
#include <iostream>
#include <string>
using namespace std;

class Lse6 {
public:
    static string getVehicle(string vehicles[], int size, int index) {
        if (index >= 0 && index < size) {
            return vehicles[index];
        }
        return "Invalid index";
    }
    
    static void printAllVehicles(string vehicles[], int size) {
        for (int i = 0; i < size; i++) {
            cout << vehicles[i] << endl;
        }
    }
};

int main() {
    string vehicles[] = {"Car", "Truck", "Motorcycle", "Bus", "Bicycle"};
    int size = 5;
    
    // Test cases
    cout << "Test Case 1: Index 0" << endl;
    cout << "Vehicle: " << Lse6::getVehicle(vehicles, size, 0) << endl;
    cout << endl;
    
    cout << "Test Case 2: Index 2" << endl;
    cout << "Vehicle: " << Lse6::getVehicle(vehicles, size, 2) << endl;
    cout << endl;
    
    cout << "Test Case 3: Index 4" << endl;
    cout << "Vehicle: " << Lse6::getVehicle(vehicles, size, 4) << endl;
    cout << endl;
    
    cout << "Test Case 4: Invalid index -1" << endl;
    cout << "Vehicle: " << Lse6::getVehicle(vehicles, size, -1) << endl;
    cout << endl;
    
    cout << "Test Case 5: Invalid index 10" << endl;
    cout << "Vehicle: " << Lse6::getVehicle(vehicles, size, 10) << endl;
    cout << endl;
    
    cout << "All vehicles:" << endl;
    Lse6::printAllVehicles(vehicles, size);
    
    return 0;
}
