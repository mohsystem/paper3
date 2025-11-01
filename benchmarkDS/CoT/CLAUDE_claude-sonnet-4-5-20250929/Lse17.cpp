
#include <iostream>
using namespace std;

class Lse17 {
private:
    static const int ARRAY_SIZE = 10;
    static const int dataArray[ARRAY_SIZE];
    
public:
    static int getValueAtIndex(int index) {
        // Input validation - check if index is within valid bounds
        if (index < 0 || index >= ARRAY_SIZE) {
            cout << "Error: Index out of bounds. Valid range: 0-" << (ARRAY_SIZE - 1) << endl;
            return -1; // Return error value
        }
        return dataArray[index];
    }
};

// Initialize static array
const int Lse17::dataArray[Lse17::ARRAY_SIZE] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

int main() {
    // Test case 1: Valid index at beginning
    cout << "Test 1 (index 0): " << Lse17::getValueAtIndex(0) << endl;
    
    // Test case 2: Valid index in middle
    cout << "Test 2 (index 5): " << Lse17::getValueAtIndex(5) << endl;
    
    // Test case 3: Valid index at end
    cout << "Test 3 (index 9): " << Lse17::getValueAtIndex(9) << endl;
    
    // Test case 4: Invalid negative index
    cout << "Test 4 (index -1): " << Lse17::getValueAtIndex(-1) << endl;
    
    // Test case 5: Invalid index beyond array
    cout << "Test 5 (index 15): " << Lse17::getValueAtIndex(15) << endl;
    
    return 0;
}
