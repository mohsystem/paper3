
#include <iostream>
#include <vector>
using namespace std;

int findMissingNumber(const vector<int>& arr, int n) {
    // Calculate expected sum of 1 to n
    long long expectedSum = (long long)n * (n + 1) / 2;
    
    // Calculate actual sum of array elements
    long long actualSum = 0;
    for (int num : arr) {
        actualSum += num;
    }
    
    // Missing number is the difference
    return expectedSum - actualSum;
}

int main() {
    // Test case 1: Missing number is 5
    vector<int> test1 = {1, 2, 3, 4, 6, 7, 8, 9, 10};
    cout << "Test 1 - Missing number: " << findMissingNumber(test1, 10) << endl;
    
    // Test case 2: Missing number is 1
    vector<int> test2 = {2, 3, 4, 5};
    cout << "Test 2 - Missing number: " << findMissingNumber(test2, 5) << endl;
    
    // Test case 3: Missing number is 10
    vector<int> test3 = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    cout << "Test 3 - Missing number: " << findMissingNumber(test3, 10) << endl;
    
    // Test case 4: Missing number is 3
    vector<int> test4 = {1, 2, 4, 5, 6};
    cout << "Test 4 - Missing number: " << findMissingNumber(test4, 6) << endl;
    
    // Test case 5: Missing number is 7
    vector<int> test5 = {1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12};
    cout << "Test 5 - Missing number: " << findMissingNumber(test5, 12) << endl;
    
    return 0;
}
