
#include <iostream>
#include <stdexcept>
using namespace std;

int id_sequence[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
int id_sequence_size = 10;

int getValueAtIndex(int index) {
    if (index >= 0 && index < id_sequence_size) {
        return id_sequence[index];
    }
    throw out_of_range("Index out of bounds");
}

int main() {
    // Test case 1: Valid index at beginning
    try {
        cout << "Test 1 (index 0): " << getValueAtIndex(0) << endl;
    } catch (const exception& e) {
        cout << "Test 1 failed: " << e.what() << endl;
    }
    
    // Test case 2: Valid index in middle
    try {
        cout << "Test 2 (index 5): " << getValueAtIndex(5) << endl;
    } catch (const exception& e) {
        cout << "Test 2 failed: " << e.what() << endl;
    }
    
    // Test case 3: Valid index at end
    try {
        cout << "Test 3 (index 9): " << getValueAtIndex(9) << endl;
    } catch (const exception& e) {
        cout << "Test 3 failed: " << e.what() << endl;
    }
    
    // Test case 4: Invalid negative index
    try {
        cout << "Test 4 (index -1): " << getValueAtIndex(-1) << endl;
    } catch (const exception& e) {
        cout << "Test 4 failed: " << e.what() << endl;
    }
    
    // Test case 5: Invalid index beyond array length
    try {
        cout << "Test 5 (index 15): " << getValueAtIndex(15) << endl;
    } catch (const exception& e) {
        cout << "Test 5 failed: " << e.what() << endl;
    }
    
    return 0;
}
