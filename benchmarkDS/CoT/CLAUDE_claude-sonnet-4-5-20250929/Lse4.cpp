
#include <iostream>
#include <string>
#include <limits>
#include <sstream>

using namespace std;

class Lse4 {
private:
    static const int VEHICLE_COUNT = 5;
    string vehicles[VEHICLE_COUNT] = {
        "Toyota Camry",
        "Honda Civic",
        "Ford F-150",
        "Tesla Model 3",
        "Chevrolet Silverado"
    };

public:
    string getVehicleAtIndex(int index) {
        // Validate index bounds
        if (index < 0 || index >= VEHICLE_COUNT) {
            return "Error: Index out of bounds. Valid range: 0 to " + to_string(VEHICLE_COUNT - 1);
        }
        return vehicles[index];
    }
    
    void runTests() {
        cout << "=== Test Cases ===" << endl;
        cout << "Test 1 (index 0): " << getVehicleAtIndex(0) << endl;
        cout << "Test 2 (index 2): " << getVehicleAtIndex(2) << endl;
        cout << "Test 3 (index 4): " << getVehicleAtIndex(4) << endl;
        cout << "Test 4 (index -1): " << getVehicleAtIndex(-1) << endl;
        cout << "Test 5 (index 10): " << getVehicleAtIndex(10) << endl;
    }
    
    void runInteractive() {
        cout << "\\n=== Interactive Mode ===" << endl;
        cout << "Enter an index of a vehicle (0-" << (VEHICLE_COUNT - 1) << "): ";
        
        string input;
        getline(cin, input);
        
        // Validate input is not empty
        if (input.empty()) {
            cout << "Error: Input cannot be empty" << endl;
            return;
        }
        
        // Parse and validate integer
        stringstream ss(input);
        int index;
        
        if (!(ss >> index) || !(ss.eof())) {
            cout << "Error: Invalid input. Please enter a valid integer." << endl;
            return;
        }
        
        cout << "Result: " << getVehicleAtIndex(index) << endl;
    }
};

int main() {
    Lse4 app;
    app.runTests();
    app.runInteractive();
    
    return 0;
}
