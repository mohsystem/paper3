
#include <iostream>
#include <vector>
using namespace std;

vector<int> mergeSortedArrays(const vector<int>& arr1, const vector<int>& arr2) {
    int len1 = arr1.size();
    int len2 = arr2.size();
    vector<int> result;
    result.reserve(len1 + len2);
    
    int i = 0, j = 0;
    
    // Merge arrays while both have elements
    while (i < len1 && j < len2) {
        if (arr1[i] <= arr2[j]) {
            result.push_back(arr1[i++]);
        } else {
            result.push_back(arr2[j++]);
        }
    }
    
    // Copy remaining elements from arr1
    while (i < len1) {
        result.push_back(arr1[i++]);
    }
    
    // Copy remaining elements from arr2
    while (j < len2) {
        result.push_back(arr2[j++]);
    }
    
    return result;
}

void printArray(const vector<int>& arr) {
    for (int num : arr) {
        cout << num << " ";
    }
    cout << endl;
}

int main() {
    // Test case 1: Normal case
    vector<int> arr1 = {1, 3, 5, 7};
    vector<int> arr2 = {2, 4, 6, 8};
    vector<int> result1 = mergeSortedArrays(arr1, arr2);
    cout << "Test 1: ";
    printArray(result1);
    
    // Test case 2: One empty array
    vector<int> arr3 = {};
    vector<int> arr4 = {1, 2, 3};
    vector<int> result2 = mergeSortedArrays(arr3, arr4);
    cout << "Test 2: ";
    printArray(result2);
    
    // Test case 3: Arrays with duplicates
    vector<int> arr5 = {1, 3, 3, 5};
    vector<int> arr6 = {2, 3, 4};
    vector<int> result3 = mergeSortedArrays(arr5, arr6);
    cout << "Test 3: ";
    printArray(result3);
    
    // Test case 4: Different lengths
    vector<int> arr7 = {1, 2};
    vector<int> arr8 = {3, 4, 5, 6, 7};
    vector<int> result4 = mergeSortedArrays(arr7, arr8);
    cout << "Test 4: ";
    printArray(result4);
    
    // Test case 5: Negative numbers
    vector<int> arr9 = {-5, -3, -1};
    vector<int> arr10 = {-4, -2, 0, 2};
    vector<int> result5 = mergeSortedArrays(arr9, arr10);
    cout << "Test 5: ";
    printArray(result5);
    
    return 0;
}
