
#include <iostream>
using namespace std;

int getValueFromArray(int array[], int size, int index) {
    if (index >= 0 && index < size) {
        return array[index];
    }
    return -1; // Return -1 for invalid index
}

int main() {
    // Test case 1
    int arr1[] = {10, 20, 30, 40, 50};
    cout << "Test 1: " << getValueFromArray(arr1, 5, 2) << endl; // Expected: 30
    
    // Test case 2
    int arr2[] = {5, 15, 25, 35, 45};
    cout << "Test 2: " << getValueFromArray(arr2, 5, 0) << endl; // Expected: 5
    
    // Test case 3
    int arr3[] = {100, 200, 300, 400, 500};
    cout << "Test 3: " << getValueFromArray(arr3, 5, 4) << endl; // Expected: 500
    
    // Test case 4
    int arr4[] = {7, 14, 21, 28, 35};
    cout << "Test 4: " << getValueFromArray(arr4, 5, 3) << endl; // Expected: 28
    
    // Test case 5
    int arr5[] = {1, 2, 3, 4, 5};
    cout << "Test 5: " << getValueFromArray(arr5, 5, 1) << endl; // Expected: 2
    
    // If you want to read from standard input and use as exit code:
    // int index;
    // cin >> index;
    // int result = getValueFromArray(arr1, 5, index);
    // return result;
    
    return 0;
}
