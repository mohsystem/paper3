
#include <iostream>
#include <vector>
using namespace std;

vector<int> mergeSortedArrays(vector<int> arr1, vector<int> arr2) {
    vector<int> result;
    int i = 0, j = 0;
    
    while (i < arr1.size() && j < arr2.size()) {
        if (arr1[i] <= arr2[j]) {
            result.push_back(arr1[i++]);
        } else {
            result.push_back(arr2[j++]);
        }
    }
    
    while (i < arr1.size()) {
        result.push_back(arr1[i++]);
    }
    
    while (j < arr2.size()) {
        result.push_back(arr2[j++]);
    }
    
    return result;
}

int main() {
    // Test case 1: Normal case
    vector<int> arr1 = {1, 3, 5, 7};
    vector<int> arr2 = {2, 4, 6, 8};
    vector<int> result1 = mergeSortedArrays(arr1, arr2);
    cout << "Test 1: ";
    for (int num : result1) cout << num << " ";
    cout << endl;
    
    // Test case 2: One empty array
    vector<int> arr3 = {};
    vector<int> arr4 = {1, 2, 3};
    vector<int> result2 = mergeSortedArrays(arr3, arr4);
    cout << "Test 2: ";
    for (int num : result2) cout << num << " ";
    cout << endl;
    
    // Test case 3: Different lengths
    vector<int> arr5 = {1, 5, 9};
    vector<int> arr6 = {2, 3, 4, 6, 7, 8};
    vector<int> result3 = mergeSortedArrays(arr5, arr6);
    cout << "Test 3: ";
    for (int num : result3) cout << num << " ";
    cout << endl;
    
    // Test case 4: Duplicate elements
    vector<int> arr7 = {1, 3, 5, 5};
    vector<int> arr8 = {2, 3, 5, 6};
    vector<int> result4 = mergeSortedArrays(arr7, arr8);
    cout << "Test 4: ";
    for (int num : result4) cout << num << " ";
    cout << endl;
    
    // Test case 5: Negative numbers
    vector<int> arr9 = {-5, -3, 0, 2};
    vector<int> arr10 = {-4, -1, 1, 3};
    vector<int> result5 = mergeSortedArrays(arr9, arr10);
    cout << "Test 5: ";
    for (int num : result5) cout << num << " ";
    cout << endl;
    
    return 0;
}
