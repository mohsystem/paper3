
#include <iostream>
#include <fstream>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <vector>

using namespace std;

int binarySearch(int arr[], int size, int target) {
    int left = 0;
    int right = size - 1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        
        if (arr[mid] == target) {
            return mid;
        } else if (arr[mid] < target) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    return -1;
}

void writeExecutionTime(long long executionTime) {
    char tempFileName[256];
    sprintf(tempFileName, "/tmp/binary_search_execution_%ld.txt", time(NULL));
    
    ofstream outFile(tempFileName);
    if (outFile.is_open()) {
        outFile << "Binary Search Execution Time: " << executionTime << " nanoseconds" << endl;
        outFile << "Execution Time: " << (executionTime / 1000000.0) << " milliseconds" << endl;
        outFile.close();
        cout << "Execution time written to: " << tempFileName << endl;
    } else {
        cout << "Error writing to file" << endl;
    }
}

void printArray(int arr[], int size) {
    cout << "[";
    for (int i = 0; i < size; i++) {
        cout << arr[i];
        if (i < size - 1) cout << ", ";
    }
    cout << "]";
}

int main() {
    // Test Case 1
    int arr1[] = {1, 3, 5, 7, 9, 11, 13, 15, 17, 19};
    int size1 = sizeof(arr1) / sizeof(arr1[0]);
    int target1 = 7;
    auto start1 = chrono::high_resolution_clock::now();
    int result1 = binarySearch(arr1, size1, target1);
    auto end1 = chrono::high_resolution_clock::now();
    long long executionTime1 = chrono::duration_cast<chrono::nanoseconds>(end1 - start1).count();
    cout << "Test 1: Array: ";
    printArray(arr1, size1);
    cout << ", Target: " << target1 << ", Result: " << result1 << endl;
    writeExecutionTime(executionTime1);
    
    // Test Case 2
    int arr2[] = {2, 4, 6, 8, 10, 12, 14, 16, 18, 20};
    int size2 = sizeof(arr2) / sizeof(arr2[0]);
    int target2 = 20;
    auto start2 = chrono::high_resolution_clock::now();
    int result2 = binarySearch(arr2, size2, target2);
    auto end2 = chrono::high_resolution_clock::now();
    long long executionTime2 = chrono::duration_cast<chrono::nanoseconds>(end2 - start2).count();
    cout << "Test 2: Array: ";
    printArray(arr2, size2);
    cout << ", Target: " << target2 << ", Result: " << result2 << endl;
    writeExecutionTime(executionTime2);
    
    // Test Case 3
    int arr3[] = {5, 10, 15, 20, 25, 30, 35, 40};
    int size3 = sizeof(arr3) / sizeof(arr3[0]);
    int target3 = 100;
    auto start3 = chrono::high_resolution_clock::now();
    int result3 = binarySearch(arr3, size3, target3);
    auto end3 = chrono::high_resolution_clock::now();
    long long executionTime3 = chrono::duration_cast<chrono::nanoseconds>(end3 - start3).count();
    cout << "Test 3: Array: ";
    printArray(arr3, size3);
    cout << ", Target: " << target3 << ", Result: " << result3 << endl;
    writeExecutionTime(executionTime3);
    
    // Test Case 4
    int arr4[] = {1, 2, 3, 4, 5};
    int size4 = sizeof(arr4) / sizeof(arr4[0]);
    int target4 = 1;
    auto start4 = chrono::high_resolution_clock::now();
    int result4 = binarySearch(arr4, size4, target4);
    auto end4 = chrono::high_resolution_clock::now();
    long long executionTime4 = chrono::duration_cast<chrono::nanoseconds>(end4 - start4).count();
    cout << "Test 4: Array: ";
    printArray(arr4, size4);
    cout << ", Target: " << target4 << ", Result: " << result4 << endl;
    writeExecutionTime(executionTime4);
    
    // Test Case 5
    int arr5[] = {11, 22, 33, 44, 55, 66, 77, 88, 99};
    int size5 = sizeof(arr5) / sizeof(arr5[0]);
    int target5 = 55;
    auto start5 = chrono::high_resolution_clock::now();
    int result5 = binarySearch(arr5, size5, target5);
    auto end5 = chrono::high_resolution_clock::now();
    long long executionTime5 = chrono::duration_cast<chrono::nanoseconds>(end5 - start5).count();
    cout << "Test 5: Array: ";
    printArray(arr5, size5);
    cout << ", Target: " << target5 << ", Result: " << result5 << endl;
    writeExecutionTime(executionTime5);
    
    return 0;
}
