
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
    // Test case 1
    vector<int> arr1 = {12, 11, 13, 5, 6, 7};
    cout << "Test 1 - Original: ";
    printArray(arr1);
    heapSort(arr1);
    cout << "Test 1 - Sorted: ";
    printArray(arr1);
    
    // Test case 2
    vector<int> arr2 = {64, 34, 25, 12, 22, 11, 90};
    cout << "\\nTest 2 - Original: ";
    printArray(arr2);
    heapSort(arr2);
    cout << "Test 2 - Sorted: ";
    printArray(arr2);
    
    // Test case 3
    vector<int> arr3 = {5, 4, 3, 2, 1};
    cout << "\\nTest 3 - Original: ";
    printArray(arr3);
    heapSort(arr3);
    cout << "Test 3 - Sorted: ";
    printArray(arr3);
    
    // Test case 4
    vector<int> arr4 = {1};
    cout << "\\nTest 4 - Original: ";
    printArray(arr4);
    heapSort(arr4);
    cout << "Test 4 - Sorted: ";
    printArray(arr4);
    
    // Test case 5
    vector<int> arr5 = {-5, 10, -3, 0, 8, -20, 15};
    cout << "\\nTest 5 - Original: ";
    printArray(arr5);
    heapSort(arr5);
    cout << "Test 5 - Sorted: ";
    printArray(arr5);
    
    return 0;
}
