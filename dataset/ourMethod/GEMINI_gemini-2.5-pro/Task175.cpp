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

// This is a dummy interface for compilation and testing purposes.
class MountainArray {
  public:
    virtual int get(int index) = 0;
    virtual int length() = 0;
    virtual ~MountainArray() = default;
};

// Dummy implementation for local testing.
class MountainArrayImpl : public MountainArray {
private:
    std::vector<int> arr;

public:
    MountainArrayImpl(const std::vector<int>& data) : arr(data) {}

    int get(int index) override {
        return arr[index];
    }

    int length() override {
        return arr.size();
    }
};

int findInMountainArray(int target, MountainArray &mountainArr) {
    int length = mountainArr.length();

    // 1. Find the index of the peak element.
    int low = 0;
    int high = length - 1;
    while (low < high) {
        int mid = low + (high - low) / 2;
        if (mountainArr.get(mid) < mountainArr.get(mid + 1)) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }
    int peakIndex = low;

    // 2. Search in the left (strictly increasing) part of the array.
    low = 0;
    high = peakIndex;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        int val = mountainArr.get(mid);
        if (val == target) {
            return mid;
        } else if (val < target) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    // 3. If not found, search in the right (strictly decreasing) part.
    low = peakIndex + 1;
    high = length - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        int val = mountainArr.get(mid);
        if (val == target) {
            return mid;
        } else if (val < target) { // In a decreasing array, smaller values are to the right.
            high = mid - 1;
        } else {
            low = mid + 1;
        }
    }

    // 4. Target not found in the array.
    return -1;
}

int main() {
    // Test Case 1
    MountainArrayImpl mountainArr1({1, 2, 3, 4, 5, 3, 1});
    int target1 = 3;
    std::cout << "Test 1 Output: " << findInMountainArray(target1, mountainArr1) << std::endl; // Expected: 2

    // Test Case 2
    MountainArrayImpl mountainArr2({0, 1, 2, 4, 2, 1});
    int target2 = 3;
    std::cout << "Test 2 Output: " << findInMountainArray(target2, mountainArr2) << std::endl; // Expected: -1

    // Test Case 3
    MountainArrayImpl mountainArr3({1, 5, 2});
    int target3 = 2;
    std::cout << "Test 3 Output: " << findInMountainArray(target3, mountainArr3) << std::endl; // Expected: 2

    // Test Case 4
    MountainArrayImpl mountainArr4({3, 5, 3, 2, 0});
    int target4 = 0;
    std::cout << "Test 4 Output: " << findInMountainArray(target4, mountainArr4) << std::endl; // Expected: 4

    // Test Case 5
    MountainArrayImpl mountainArr5({1, 5, 2});
    int target5 = 5;
    std::cout << "Test 5 Output: " << findInMountainArray(target5, mountainArr5) << std::endl; // Expected: 1

    return 0;
}