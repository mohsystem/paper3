
#include <iostream>
#include <vector>
using namespace std;

int findMissingNumber(int arr[], int size, int n) {
    int expectedSum = n * (n + 1) / 2;
    int actualSum = 0;
    for (int i = 0; i < size; i++) {
        actualSum += arr[i];
    }
    return expectedSum - actualSum;
}

int main() {
    // Test case 1
    int test1[] = {1, 2, 4, 5, 6};
    cout << "Test 1 - Missing number: " << findMissingNumber(test1, 5, 6) << endl;
    
    // Test case 2
    int test2[] = {1, 2, 3, 4, 5, 6, 7, 9, 10};
    cout << "Test 2 - Missing number: " << findMissingNumber(test2, 9, 10) << endl;
    
    // Test case 3
    int test3[] = {2, 3, 4, 5};
    cout << "Test 3 - Missing number: " << findMissingNumber(test3, 4, 5) << endl;
    
    // Test case 4
    int test4[] = {1};
    cout << "Test 4 - Missing number: " << findMissingNumber(test4, 1, 2) << endl;
    
    // Test case 5
    int test5[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 16};
    cout << "Test 5 - Missing number: " << findMissingNumber(test5, 15, 16) << endl;
    
    return 0;
}
