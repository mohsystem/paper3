#include <iostream>
#include <vector>
#include <numeric>

int findEvenIndex(const std::vector<int>& arr) {
    long long totalSum = std::accumulate(arr.begin(), arr.end(), 0LL);
    
    long long leftSum = 0;
    for (size_t i = 0; i < arr.size(); ++i) {
        // The right sum is the total sum minus the left sum and the current element
        long long rightSum = totalSum - leftSum - arr[i];
        if (leftSum == rightSum) {
            return static_cast<int>(i);
        }
        leftSum += arr[i];
    }
    
    return -1;
}

void printVector(const std::vector<int>& vec) {
    std::cout << "{";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << (i == vec.size() - 1 ? "" : ", ");
    }
    std::cout << "}";
}

int main() {
    // Test Case 1
    std::vector<int> arr1 = {1, 2, 3, 4, 3, 2, 1};
    std::cout << "Array: ";
    printVector(arr1);
    std::cout << " -> Index: " << findEvenIndex(arr1) << std::endl;

    // Test Case 2
    std::vector<int> arr2 = {1, 100, 50, -51, 1, 1};
    std::cout << "Array: ";
    printVector(arr2);
    std::cout << " -> Index: " << findEvenIndex(arr2) << std::endl;

    // Test Case 3
    std::vector<int> arr3 = {20, 10, -80, 10, 10, 15, 35};
    std::cout << "Array: ";
    printVector(arr3);
    std::cout << " -> Index: " << findEvenIndex(arr3) << std::endl;
    
    // Test Case 4
    std::vector<int> arr4 = {10, -80, 10, 10, 15, 35, 20};
    std::cout << "Array: ";
    printVector(arr4);
    std::cout << " -> Index: " << findEvenIndex(arr4) << std::endl;
    
    // Test Case 5
    std::vector<int> arr5 = {1, 2, 3, 4, 5, 6};
    std::cout << "Array: ";
    printVector(arr5);
    std::cout << " -> Index: " << findEvenIndex(arr5) << std::endl;
    
    return 0;
}