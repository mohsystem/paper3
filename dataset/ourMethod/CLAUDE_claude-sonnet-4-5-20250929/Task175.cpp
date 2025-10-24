
#include <vector>
#include <stdexcept>
#include <iostream>
#include <limits>

// MountainArray interface for interactive problem
class MountainArray {
private:
    std::vector<int> arr;
    int callCount;
    static const int MAX_CALLS = 100;
    
public:
    explicit MountainArray(const std::vector<int>& array) : arr(array), callCount(0) {
        // Validate mountain array properties
        if (array.size() < 3) {
            throw std::invalid_argument("Array length must be >= 3");
        }
    }
    
    int get(int index) {
        // Validate index bounds to prevent out-of-bounds access
        if (index < 0 || index >= static_cast<int>(arr.size())) {
            throw std::out_of_range("Index out of bounds");
        }
        // Track API calls to ensure we don't exceed limit
        if (++callCount > MAX_CALLS) {
            throw std::runtime_error("Exceeded maximum API calls");
        }
        return arr[index];
    }
    
    int length() const {
        return static_cast<int>(arr.size());
    }
    
    int getCallCount() const {
        return callCount;
    }
};

class Solution {
private:
    // Binary search to find peak index in mountain array
    // Security: all indices validated before access
    int findPeak(MountainArray& mountainArr) {
        int left = 0;
        int length = mountainArr.length();
        int right = length - 1;
        
        // Validate length to prevent integer overflow
        if (length <= 0 || length > 10000) {
            return -1;
        }
        
        while (left < right) {
            // Prevent integer overflow in mid calculation
            int mid = left + (right - left) / 2;
            
            // Validate mid is in valid range
            if (mid < 0 || mid >= length - 1) {
                break;
            }
            
            int midVal = mountainArr.get(mid);
            int nextVal = mountainArr.get(mid + 1);
            
            if (midVal < nextVal) {
                left = mid + 1;
            } else {
                right = mid;
            }
        }
        
        return left;
    }
    
    // Binary search in ascending part of mountain
    // Security: bounds checked, prevents overflow
    int binarySearchAsc(MountainArray& mountainArr, int target, int left, int right) {
        // Validate bounds
        if (left < 0 || right >= mountainArr.length() || left > right) {
            return -1;
        }
        
        while (left <= right) {
            // Prevent overflow in mid calculation
            int mid = left + (right - left) / 2;
            
            // Validate mid before access
            if (mid < 0 || mid >= mountainArr.length()) {
                return -1;
            }
            
            int midVal = mountainArr.get(mid);
            
            if (midVal == target) {
                return mid;
            } else if (midVal < target) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
        
        return -1;
    }
    
    // Binary search in descending part of mountain
    // Security: bounds checked, prevents overflow
    int binarySearchDesc(MountainArray& mountainArr, int target, int left, int right) {
        // Validate bounds
        if (left < 0 || right >= mountainArr.length() || left > right) {
            return -1;
        }
        
        while (left <= right) {
            // Prevent overflow in mid calculation
            int mid = left + (right - left) / 2;
            
            // Validate mid before access
            if (mid < 0 || mid >= mountainArr.length()) {
                return -1;
            }
            
            int midVal = mountainArr.get(mid);
            
            if (midVal == target) {
                return mid;
            } else if (midVal > target) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
        
        return -1;
    }
    
public:
    int findInMountainArray(int target, MountainArray& mountainArr) {
        // Validate input constraints
        int length = mountainArr.length();
        if (length < 3 || length > 10000) {
            return -1;
        }
        
        // Validate target range
        if (target < 0 || target > 1000000000) {
            return -1;
        }
        
        // Find peak of mountain
        int peak = findPeak(mountainArr);
        
        // Validate peak index
        if (peak < 0 || peak >= length) {
            return -1;
        }
        
        // Search in ascending part first (to get minimum index)
        int result = binarySearchAsc(mountainArr, target, 0, peak);
        
        if (result != -1) {
            return result;
        }
        
        // Search in descending part if not found
        return binarySearchDesc(mountainArr, target, peak + 1, length - 1);
    }
};

int main() {
    Solution solution;
    
    // Test case 1: target exists at multiple positions
    try {
        std::vector<int> arr1 = {1, 2, 3, 4, 5, 3, 1};
        MountainArray mountainArr1(arr1);
        int result1 = solution.findInMountainArray(3, mountainArr1);
        std::cout << "Test 1: " << result1 << " (expected: 2)" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 1 failed: " << e.what() << std::endl;
    }
    
    // Test case 2: target does not exist
    try {
        std::vector<int> arr2 = {0, 1, 2, 4, 2, 1};
        MountainArray mountainArr2(arr2);
        int result2 = solution.findInMountainArray(3, mountainArr2);
        std::cout << "Test 2: " << result2 << " (expected: -1)" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 2 failed: " << e.what() << std::endl;
    }
    
    // Test case 3: target at peak
    try {
        std::vector<int> arr3 = {1, 2, 3, 4, 5, 3, 1};
        MountainArray mountainArr3(arr3);
        int result3 = solution.findInMountainArray(5, mountainArr3);
        std::cout << "Test 3: " << result3 << " (expected: 4)" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 3 failed: " << e.what() << std::endl;
    }
    
    // Test case 4: target at beginning
    try {
        std::vector<int> arr4 = {1, 2, 3, 4, 5, 3, 1};
        MountainArray mountainArr4(arr4);
        int result4 = solution.findInMountainArray(1, mountainArr4);
        std::cout << "Test 4: " << result4 << " (expected: 0)" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 4 failed: " << e.what() << std::endl;
    }
    
    // Test case 5: large mountain array
    try {
        std::vector<int> arr5 = {1, 5, 2};
        MountainArray mountainArr5(arr5);
        int result5 = solution.findInMountainArray(2, mountainArr5);
        std::cout << "Test 5: " << result5 << " (expected: 2)" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 5 failed: " << e.what() << std::endl;
    }
    
    return 0;
}
