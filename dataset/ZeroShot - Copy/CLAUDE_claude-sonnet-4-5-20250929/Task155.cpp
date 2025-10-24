
#include <iostream>
#include <vector>
using namespace std;

void heapify(vector<int>& arr, int n, int i) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;
    
    // If left child is larger than root
    if (left < n && arr[left] > arr[largest]) {
        largest = left;
    }
    
    // If right child is larger than largest so far
    if (right < n && arr[right] > arr[largest]) {
        largest = right;
    }
    
    // If largest is not root
    if (largest != i) {
        swap(arr[i], arr[largest]);
        
        // Recursively heapify the affected sub-tree
        heapify(arr, n, largest);
    }
}

void heapSort(vector<int>& arr) {
    if (arr.empty() || arr.size() <= 1) {
        return;
    }
    
    int n = arr.size();
    
    // Build max heap
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(arr, n, i);
    }
    
    // Extract elements from heap one by one
    for (int i = n - 1; i > 0; i--) {
        // Move current root to end
        swap(arr[0], arr[i]);
        
        // Heapify the reduced heap
        heapify(arr, i, 0);
    }
}

void printArray(const vector<int>& arr) {
    cout << "[";
    for (size_t i = 0; i < arr.size(); i++) {
        cout << arr[i];
        if (i < arr.size() - 1) cout << ", ";
    }
    cout << "]" << endl;
}

int main() {
    // Test case 1: Regular unsorted array
    vector<int> arr1 = {12, 11, 13, 5, 6, 7};
    heapSort(arr1);
    cout << "Test 1: ";
    printArray(arr1);
    
    // Test case 2: Already sorted array
    vector<int> arr2 = {1, 2, 3, 4, 5};
    heapSort(arr2);
    cout << "Test 2: ";
    printArray(arr2);
    
    // Test case 3: Reverse sorted array
    vector<int> arr3 = {9, 7, 5, 3, 1};
    heapSort(arr3);
    cout << "Test 3: ";
    printArray(arr3);
    
    // Test case 4: Array with duplicates
    vector<int> arr4 = {5, 2, 8, 2, 9, 1, 5};
    heapSort(arr4);
    cout << "Test 4: ";
    printArray(arr4);
    
    // Test case 5: Single element array
    vector<int> arr5 = {42};
    heapSort(arr5);
    cout << "Test 5: ";
    printArray(arr5);
    
    return 0;
}
