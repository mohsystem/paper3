
#include <iostream>
#include <limits>
using namespace std;

const int ID_SEQUENCE[] = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};
const int ARRAY_SIZE = sizeof(ID_SEQUENCE) / sizeof(ID_SEQUENCE[0]);

int getValueAtIndex(int index, bool& success) {
    success = false;
    
    // Input validation: check for negative index
    if (index < 0) {
        cout << "Error: Index cannot be negative" << endl;
        return -1;
    }
    
    // Boundary check: ensure index is within array bounds
    if (index >= ARRAY_SIZE) {
        cout << "Error: Index out of bounds" << endl;
        return -1;
    }
    
    success = true;
    return ID_SEQUENCE[index];
}

void testCase(int index, const char* description) {
    cout << "\\nTest: " << description << endl;
    bool success = false;
    int result = getValueAtIndex(index, success);
    if (success) {
        cout << "Result: " << result << endl;
    }
}

int main() {
    // Test cases
    cout << "=== Running Test Cases ===" << endl;
    testCase(0, "Valid index 0");
    testCase(5, "Valid index 5");
    testCase(9, "Valid index at boundary");
    testCase(-1, "Invalid negative index");
    testCase(15, "Invalid index out of bounds");
    
    // Interactive mode
    cout << "\\n=== Interactive Mode ===" << endl;
    cout << "Enter an index value: ";
    
    int index;
    if (cin >> index) {
        bool success = false;
        int result = getValueAtIndex(index, success);
        if (success) {
            cout << "Value at index " << index << ": " << result << endl;
        }
    } else {
        cout << "Error: Invalid input. Please enter an integer." << endl;
    }
    
    return 0;
}
