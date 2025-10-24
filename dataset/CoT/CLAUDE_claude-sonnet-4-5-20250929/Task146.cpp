
#include <iostream>
#include <vector>
using namespace std;

/**
 * Finds the missing number in an array containing unique integers from 1 to n
 * @param arr Vector of integers with one missing number
 * @return The missing number
 */
int findMissingNumber(const vector<int>& arr) {
    if (arr.empty()) {
        return 1;
    }
    
    int n = arr.size() + 1;
    // Using mathematical formula: sum of 1 to n = n*(n+1)/2
    long long expectedSum = (long long)n * (n + 1) / 2;
    long long actualSum = 0;
    
    for (int num : arr) {
        actualSum += num;
    }
    
    return (int)(expectedSum - actualSum);
}

void printArray(const vector<int>& arr) {
    cout << "[";
    for (size_t i = 0; i < arr.size(); i++) {
        cout << arr[i];
        if (i < arr.size() - 1) cout << ", ";
    }
    cout << "]";
}

int main() {
    // Test case 1: Missing number is 5
    vector<int> test1 = {1, 2, 3, 4, 6};
    cout << "Test 1 - Array: ";
    printArray(test1);
    cout << "\\nMissing number: " << findMissingNumber(test1) << "\\n\\n";
    
    // Test case 2: Missing number is 1
    vector<int> test2 = {2, 3, 4, 5, 6};
    cout << "Test 2 - Array: ";
    printArray(test2);
    cout << "\\nMissing number: " << findMissingNumber(test2) << "\\n\\n";
    
    // Test case 3: Missing number is 10
    vector<int> test3 = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    cout << "Test 3 - Array: ";
    printArray(test3);
    cout << "\\nMissing number: " << findMissingNumber(test3) << "\\n\\n";
    
    // Test case 4: Missing number is 3
    vector<int> test4 = {1, 2, 4, 5, 6, 7};
    cout << "Test 4 - Array: ";
    printArray(test4);
    cout << "\\nMissing number: " << findMissingNumber(test4) << "\\n\\n";
    
    // Test case 5: Array with single element missing (n=2)
    vector<int> test5 = {2};
    cout << "Test 5 - Array: ";
    printArray(test5);
    cout << "\\nMissing number: " << findMissingNumber(test5) << "\\n\\n";
    
    return 0;
}
