
#include <iostream>
#include <limits>
using namespace std;

class Lse14 {
private:
    static const int ARRAY_SIZE = 10;
    static int id_sequence[ARRAY_SIZE];
    
public:
    static int* getValueAtIndex(int index) {
        // Validate index bounds
        if (index < 0 || index >= ARRAY_SIZE) {
            cout << "Error: Index out of bounds. Valid range: 0-" << (ARRAY_SIZE - 1) << endl;
            return nullptr;
        }
        return &id_sequence[index];
    }
    
    static int* getUserInputAndGetValue() {
        int index;
        cout << "Enter an index value (0-" << (ARRAY_SIZE - 1) << "): ";
        
        if (!(cin >> index)) {
            cout << "Error: Invalid input. Please enter a valid integer." << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\\n');
            return nullptr;
        }
        
        return getValueAtIndex(index);
    }
    
    static void runTests() {
        // Test case 1: Valid index at beginning
        int* result1 = getValueAtIndex(0);
        cout << "Test 1 - Index 0: " << (result1 ? to_string(*result1) : "null") << endl;
        
        // Test case 2: Valid index in middle
        int* result2 = getValueAtIndex(5);
        cout << "Test 2 - Index 5: " << (result2 ? to_string(*result2) : "null") << endl;
        
        // Test case 3: Valid index at end
        int* result3 = getValueAtIndex(9);
        cout << "Test 3 - Index 9: " << (result3 ? to_string(*result3) : "null") << endl;
        
        // Test case 4: Invalid negative index
        int* result4 = getValueAtIndex(-1);
        cout << "Test 4 - Index -1: " << (result4 ? to_string(*result4) : "null") << endl;
        
        // Test case 5: Invalid index beyond bounds
        int* result5 = getValueAtIndex(15);
        cout << "Test 5 - Index 15: " << (result5 ? to_string(*result5) : "null") << endl;
    }
};

int Lse14::id_sequence[ARRAY_SIZE] = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};

int main() {
    Lse14::runTests();
    return 0;
}
