
#include <iostream>
#include <vector>
using namespace std;

vector<int> mergeSortedArrays(vector<int> arr1, vector<int> arr2) {
    int n1 = arr1.size();
    int n2 = arr2.size();
    vector<int> result;
    
    int i = 0, j = 0;
    
    while (i < n1 && j < n2) {
        if (arr1[i] <= arr2[j]) {
            result.push_back(arr1[i++]);
        } else {
            result.push_back(arr2[j++]);
        }
    }
    
    while (i < n1) {
        result.push_back(arr1[i++]);
    }
    
    while (j < n2) {
        result.push_back(arr2[j++]);
    }
    
    return result;
}

void printArray(vector<int> arr) {
    cout << "[";
    for (int i = 0; i < arr.size(); i++) {
        cout << arr[i];
        if (i < arr.size() - 1) cout << ", ";
    }
    cout << "]" << endl;
}

int main() {
    // Test case 1
    vector<int> arr1_1 = {1, 3, 5, 7};
    vector<int> arr2_1 = {2, 4, 6, 8};
    cout << "Test 1: ";
    printArray(mergeSortedArrays(arr1_1, arr2_1));
    
    // Test case 2
    vector<int> arr1_2 = {1, 2, 3};
    vector<int> arr2_2 = {4, 5, 6};
    cout << "Test 2: ";
    printArray(mergeSortedArrays(arr1_2, arr2_2));
    
    // Test case 3
    vector<int> arr1_3 = {1, 5, 9};
    vector<int> arr2_3 = {2, 3, 4, 6, 7, 8};
    cout << "Test 3: ";
    printArray(mergeSortedArrays(arr1_3, arr2_3));
    
    // Test case 4
    vector<int> arr1_4 = {};
    vector<int> arr2_4 = {1, 2, 3};
    cout << "Test 4: ";
    printArray(mergeSortedArrays(arr1_4, arr2_4));
    
    // Test case 5
    vector<int> arr1_5 = {-5, -2, 0, 3};
    vector<int> arr2_5 = {-3, -1, 2, 4, 6};
    cout << "Test 5: ";
    printArray(mergeSortedArrays(arr1_5, arr2_5));
    
    return 0;
}
