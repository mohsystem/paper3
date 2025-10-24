
#include <vector>
#include <iostream>
#include <algorithm>
#include <stdexcept>

// Security: Input validation to prevent out-of-bounds access
// Constraint check: 1 <= nums.length <= 10^5, -10^4 <= nums[i] <= 10^4
void validateInput(const std::vector<int>& nums) {
    if (nums.empty() || nums.size() > 100000) {
        throw std::invalid_argument("Array size must be between 1 and 100000");
    }
    for (int num : nums) {
        if (num < -10000 || num > 10000) {
            throw std::invalid_argument("Array elements must be between -10000 and 10000");
        }
    }
}

// Merge sort approach to count smaller elements to the right
// Security: All array accesses are bounds-checked through vector operations
void mergeSort(std::vector<std::pair<int, int>>& arr, int left, int right, std::vector<int>& counts) {
    if (left >= right) return;
    
    int mid = left + (right - left) / 2; // Prevent integer overflow
    
    mergeSort(arr, left, mid, counts);
    mergeSort(arr, mid + 1, right, counts);
    
    // Merge and count
    std::vector<std::pair<int, int>> temp;
    temp.reserve(right - left + 1); // Pre-allocate to prevent reallocation
    
    int i = left, j = mid + 1;
    int rightCount = 0;
    
    while (i <= mid && j <= right) {
        if (arr[j].first < arr[i].first) {
            rightCount++;
            temp.push_back(arr[j]);
            j++;
        } else {
            counts[arr[i].second] += rightCount;
            temp.push_back(arr[i]);
            i++;
        }
    }
    
    while (i <= mid) {
        counts[arr[i].second] += rightCount;
        temp.push_back(arr[i]);
        i++;
    }
    
    while (j <= right) {
        temp.push_back(arr[j]);
        j++;
    }
    
    // Copy back - bounds checked by vector operations
    for (size_t k = 0; k < temp.size(); k++) {
        arr[left + k] = temp[k];
    }
}

std::vector<int> countSmaller(const std::vector<int>& nums) {
    // Security: Validate all inputs before processing
    validateInput(nums);
    
    size_t n = nums.size();
    std::vector<int> counts(n, 0); // Initialize with zeros
    
    // Create pairs of (value, original_index)
    std::vector<std::pair<int, int>> arr;
    arr.reserve(n); // Pre-allocate to prevent multiple reallocations
    
    for (size_t i = 0; i < n; i++) {
        arr.push_back({nums[i], static_cast<int>(i)});
    }
    
    // Security: Cast is safe as n is validated to be <= 100000
    mergeSort(arr, 0, static_cast<int>(n) - 1, counts);
    
    return counts;
}

int main() {
    // Test case 1
    std::vector<int> test1 = {5, 2, 6, 1};
    std::vector<int> result1 = countSmaller(test1);
    std::cout << "Test 1: [";
    for (size_t i = 0; i < result1.size(); i++) {
        std::cout << result1[i];
        if (i < result1.size() - 1) std::cout << ",";
    }
    std::cout << "]" << std::endl;
    
    // Test case 2
    std::vector<int> test2 = {-1};
    std::vector<int> result2 = countSmaller(test2);
    std::cout << "Test 2: [";
    for (size_t i = 0; i < result2.size(); i++) {
        std::cout << result2[i];
        if (i < result2.size() - 1) std::cout << ",";
    }
    std::cout << "]" << std::endl;
    
    // Test case 3
    std::vector<int> test3 = {-1, -1};
    std::vector<int> result3 = countSmaller(test3);
    std::cout << "Test 3: [";
    for (size_t i = 0; i < result3.size(); i++) {
        std::cout << result3[i];
        if (i < result3.size() - 1) std::cout << ",";
    }
    std::cout << "]" << std::endl;
    
    // Test case 4
    std::vector<int> test4 = {1, 2, 3, 4, 5};
    std::vector<int> result4 = countSmaller(test4);
    std::cout << "Test 4: [";
    for (size_t i = 0; i < result4.size(); i++) {
        std::cout << result4[i];
        if (i < result4.size() - 1) std::cout << ",";
    }
    std::cout << "]" << std::endl;
    
    // Test case 5
    std::vector<int> test5 = {5, 4, 3, 2, 1};
    std::vector<int> result5 = countSmaller(test5);
    std::cout << "Test 5: [";
    for (size_t i = 0; i < result5.size(); i++) {
        std::cout << result5[i];
        if (i < result5.size() - 1) std::cout << ",";
    }
    std::cout << "]" << std::endl;
    
    return 0;
}
