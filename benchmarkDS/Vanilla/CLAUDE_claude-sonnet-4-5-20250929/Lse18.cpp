
#include <iostream>
#include <stdexcept>
using namespace std;

int array[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
int arraySize = 10;

int getValueAtIndex(int index) {
    if (index >= 0 && index < arraySize) {
        return array[index];
    }
    throw out_of_range("Index out of bounds");
}

int main() {
    // Test case 1
    try {
        cout << "Index 5: " << getValueAtIndex(5) << endl;
    } catch (const exception& e) {
        cout << "Error: " << e.what() << endl;
    }
    
    // Test case 2
    try {
        cout << "Index 0: " << getValueAtIndex(0) << endl;
    } catch (const exception& e) {
        cout << "Error: " << e.what() << endl;
    }
    
    // Test case 3
    try {
        cout << "Index 9: " << getValueAtIndex(9) << endl;
    } catch (const exception& e) {
        cout << "Error: " << e.what() << endl;
    }
    
    // Test case 4
    try {
        cout << "Index 3: " << getValueAtIndex(3) << endl;
    } catch (const exception& e) {
        cout << "Error: " << e.what() << endl;
    }
    
    // Test case 5
    try {
        cout << "Index 10: " << getValueAtIndex(10) << endl;
    } catch (const exception& e) {
        cout << "Error: " << e.what() << endl;
    }
    
    return 0;
}
