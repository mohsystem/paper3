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
class MountainArray {
public:
    virtual ~MountainArray() = default;
    virtual int get(int index) = 0;
    virtual int length() = 0;
};

// Dummy implementation for testing purposes
class MountainArrayImpl : public MountainArray {
private:
    std::vector<int> arr;

public:
    MountainArrayImpl(const std::vector<int>& vec) : arr(vec) {}

    int get(int index) override {
        return arr[index];
    }

    int length() override {
        return arr.size();
    }
};

class Task175 {
public:
    int findInMountainArray(int target, MountainArray &mountainArr) {
        int length = mountainArr.length();

        // 1. Find the peak index
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

        // 2. Search in the ascending part [0, peakIndex]
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

        // 3. Search in the descending part [peakIndex + 1, length - 1]
        low = peakIndex + 1;
        high = length - 1;
        while (low <= high) {
            int mid = low + (high - low) / 2;
            int val = mountainArr.get(mid);
            if (val == target) {
                return mid;
            } else if (val < target) { // descending
                high = mid - 1;
            } else {
                low = mid + 1;
            }
        }

        return -1;
    }
};


int main() {
    Task175 solution;

    // Test Case 1
    MountainArrayImpl mountainArr1({1, 2, 3, 4, 5, 3, 1});
    int target1 = 3;
    std::cout << "Test Case 1: " << solution.findInMountainArray(target1, mountainArr1) << std::endl;

    // Test Case 2
    MountainArrayImpl mountainArr2({0, 1, 2, 4, 2, 1});
    int target2 = 3;
    std::cout << "Test Case 2: " << solution.findInMountainArray(target2, mountainArr2) << std::endl;

    // Test Case 3
    MountainArrayImpl mountainArr3({1, 5, 2});
    int target3 = 2;
    std::cout << "Test Case 3: " << solution.findInMountainArray(target3, mountainArr3) << std::endl;

    // Test Case 4
    MountainArrayImpl mountainArr4({1, 2, 5, 2, 1});
    int target4 = 1;
    std::cout << "Test Case 4: " << solution.findInMountainArray(target4, mountainArr4) << std::endl;
    
    // Test Case 5
    MountainArrayImpl mountainArr5({1, 2, 5, 2, 1});
    int target5 = 5;
    std::cout << "Test Case 5: " << solution.findInMountainArray(target5, mountainArr5) << std::endl;

    return 0;
}