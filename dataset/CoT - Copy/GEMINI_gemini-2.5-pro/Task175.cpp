#include <iostream>
#include <vector>
#include <numeric>

/**
 * // This is the MountainArray's API interface.
 * // You should not implement it, or speculate about its implementation
 * class MountainArray {
 *   public:
 *     int get(int index);
 *     int length();
 * };
 */

// Mock implementation for local testing
class MountainArray {
    std::vector<int> arr;
public:
    MountainArray(const std::vector<int>& v) : arr(v) {}
    int get(int index) {
        return arr[index];
    }
    int length() {
        return arr.size();
    }
};

class Task175 {
public:
    int findInMountainArray(int target, MountainArray &mountainArr) {
        int length = mountainArr.length();

        // 1. Find the index of the peak element
        int low = 0, high = length - 1;
        int peakIndex = -1;
        while (low < high) {
            int mid = low + (high - low) / 2;
            if (mountainArr.get(mid) < mountainArr.get(mid + 1)) {
                low = mid + 1;
            } else {
                high = mid;
            }
        }
        peakIndex = low;

        // 2. Search in the strictly increasing part
        low = 0;
        high = peakIndex;
        while (low <= high) {
            int mid = low + (high - low) / 2;
            int midVal = mountainArr.get(mid);
            if (midVal == target) {
                return mid;
            } else if (midVal < target) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }

        // 3. If not found, search in the strictly decreasing part
        low = peakIndex + 1;
        high = length - 1;
        while (low <= high) {
            int mid = low + (high - low) / 2;
            int midVal = mountainArr.get(mid);
            if (midVal == target) {
                return mid;
            } else if (midVal > target) { // Note the change in logic
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }

        // 4. Target not found
        return -1;
    }
};

int main() {
    Task175 solution;

    // Test Case 1
    MountainArray mountainArr1({1, 2, 3, 4, 5, 3, 1});
    int target1 = 3;
    std::cout << "Test Case 1: " << solution.findInMountainArray(target1, mountainArr1) << std::endl; // Expected: 2

    // Test Case 2
    MountainArray mountainArr2({0, 1, 2, 4, 2, 1});
    int target2 = 3;
    std::cout << "Test Case 2: " << solution.findInMountainArray(target2, mountainArr2) << std::endl; // Expected: -1

    // Test Case 3
    MountainArray mountainArr3({1, 5, 2});
    int target3 = 2;
    std::cout << "Test Case 3: " << solution.findInMountainArray(target3, mountainArr3) << std::endl; // Expected: 2

    // Test Case 4
    MountainArray mountainArr4({1, 2, 3, 4, 5, 6, 7});
    int target4 = 5;
    std::cout << "Test Case 4: " << solution.findInMountainArray(target4, mountainArr4) << std::endl; // Expected: 4

    // Test Case 5
    MountainArray mountainArr5({7, 6, 5, 4, 3, 2, 1});
    int target5 = 5;
    std::cout << "Test Case 5: " << solution.findInMountainArray(target5, mountainArr5) << std::endl; // Expected: 2

    return 0;
}