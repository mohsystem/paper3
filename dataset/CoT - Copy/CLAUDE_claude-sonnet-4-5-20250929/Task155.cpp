
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

class Task155 {
private:
    // Heapify a subtree rooted at index i
    static void heapify(vector<int>& arr, int n, int i) {
        int largest = i;
        int left = 2 * i + 1;
        int right = 2 * i + 2;

        // Check if left child exists and is greater than root
        if (left < n && arr[left] > arr[largest]) {
            largest = left;
        }

        // Check if right child exists and is greater than largest so far
        if (right < n && arr[right] > arr[largest]) {
            largest = right;
        }

        // If largest is not root, swap and continue heapifying
        if (largest != i) {
            swap(arr[i], arr[largest]);
            heapify(arr, n, largest);
        }
    }

public:
    // Main heap sort function
    static vector<int> heapSort(const vector<int>& arr) {
        if (arr.empty() || arr.size() <= 1) {
            return arr;
        }

        // Create a copy to avoid modifying original array
        vector<int> result = arr;
        int n = result.size();

        // Build max heap
        for (int i = n / 2 - 1; i >= 0; i--) {
            heapify(result, n, i);
        }

        // Extract elements from heap one by one
        for (int i = n - 1; i > 0; i--) {
            // Move current root to end
            swap(result[0], result[i]);
            // Heapify the reduced heap
            heapify(result, i, 0);
        }

        return result;
    }

    static void printVector(const vector<int>& vec) {
        cout << "[";
        for (size_t i = 0; i < vec.size(); i++) {
            cout << vec[i];
            if (i < vec.size() - 1) cout << ", ";
        }
        cout << "]" << endl;
    }
};

int main() {
    // Test case 1: Random integers
    vector<int> test1 = {64, 34, 25, 12, 22, 11, 90};
    cout << "Test 1 - Input: ";
    Task155::printVector(test1);
    cout << "Test 1 - Output: ";
    Task155::printVector(Task155::heapSort(test1));

    // Test case 2: Already sorted array
    vector<int> test2 = {1, 2, 3, 4, 5};
    cout << "\\nTest 2 - Input: ";
    Task155::printVector(test2);
    cout << "Test 2 - Output: ";
    Task155::printVector(Task155::heapSort(test2));

    // Test case 3: Reverse sorted array
    vector<int> test3 = {9, 7, 5, 3, 1};
    cout << "\\nTest 3 - Input: ";
    Task155::printVector(test3);
    cout << "Test 3 - Output: ";
    Task155::printVector(Task155::heapSort(test3));

    // Test case 4: Array with duplicates
    vector<int> test4 = {5, 2, 8, 2, 9, 1, 5, 5};
    cout << "\\nTest 4 - Input: ";
    Task155::printVector(test4);
    cout << "Test 4 - Output: ";
    Task155::printVector(Task155::heapSort(test4));

    // Test case 5: Single element array
    vector<int> test5 = {42};
    cout << "\\nTest 5 - Input: ";
    Task155::printVector(test5);
    cout << "Test 5 - Output: ";
    Task155::printVector(Task155::heapSort(test5));

    return 0;
}
