
#include <iostream>
#include <stdexcept>
using namespace std;

const int array[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
const int arraySize = 10;

int getValueAtIndex(int index) {
    if (index >= 0 && index < arraySize) {
        return array[index];
    }
    throw out_of_range("Index out of bounds");
}

int main() {
    // Test case 1
    try {
        cout << "Test 1 - Index 0: " << getValueAtIndex(0) << endl;
    } catch (const exception& e) {
        cout << "Test 1 - Error: " << e.what() << endl;
    }
    
    // Test case 2
    try {
        cout << "Test 2 - Index 5: " << getValueAtIndex(5) << endl;
    } catch (const exception& e) {
        cout << "Test 2 - Error: " << e.what() << endl;
    }
    
    // Test case 3
    try {
        cout << "Test 3 - Index 9: " << getValueAtIndex(9) << endl;
    } catch (const exception& e) {
        cout << "Test 3 - Error: " << e.what() << endl;
    }
    
    // Test case 4
    try {
        cout << "Test 4 - Index -1: " << getValueAtIndex(-1) << endl;
    } catch (const exception& e) {
        cout << "Test 4 - Error: " << e.what() << endl;
    }
    
    // Test case 5
    try {
        cout << "Test 5 - Index 10: " << getValueAtIndex(10) << endl;
    } catch (const exception& e) {
        cout << "Test 5 - Error: " << e.what() << endl;
    }
    
    return 0;
}
