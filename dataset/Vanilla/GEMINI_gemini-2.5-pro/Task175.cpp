#include <iostream>
#include <vector>

// This is the MountainArray's API interface.
// You should not implement it, or speculate about its implementation
class MountainArray {
  public:
    virtual ~MountainArray() = default;
    virtual int get(int index) = 0;
    virtual int length() = 0;
};

// Mock implementation for local testing
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

class Task175 {
public:
    int findInMountainArray(int target, MountainArray &mountainArr) {
        int len = mountainArr.length();

        // 1. Find the index of the peak element
        int low = 0, high = len - 1;
        while (low < high) {
            int mid = low + (high - low) / 2;
            if (mountainArr.get(mid) < mountainArr.get(mid + 1)) {
                low = mid + 1;
            } else {
                high = mid;
            }
        }
        int peak = low;

        // 2. Search in the left (ascending) subarray
        low = 0;
        high = peak;
        while (low <= high) {
            int mid = low + (high - low) / 2;
            int val = mountainArr.get(mid);
            if (val < target) {
                low = mid + 1;
            } else if (val > target) {
                high = mid - 1;
            } else {
                return mid;
            }
        }

        // 3. Search in the right (descending) subarray
        low = peak + 1;
        high = len - 1;
        while (low <= high) {
            int mid = low + (high - low) / 2;
            int val = mountainArr.get(mid);
            if (val > target) { // Note the change for descending order
                low = mid + 1;
            } else if (val < target) {
                high = mid - 1;
            } else {
                return mid;
            }
        }
        
        // 4. Target not found
        return -1;
    }
};

int main() {
    Task175 solution;

    // Test Case 1
    MountainArrayImpl ma1({1, 2, 3, 4, 5, 3, 1});
    int target1 = 3;
    std::cout << "Test Case 1: " << solution.findInMountainArray(target1, ma1) << std::endl;

    // Test Case 2
    MountainArrayImpl ma2({0, 1, 2, 4, 2, 1});
    int target2 = 3;
    std::cout << "Test Case 2: " << solution.findInMountainArray(target2, ma2) << std::endl;

    // Test Case 3
    MountainArrayImpl ma3({1, 5, 2});
    int target3 = 2;
    std::cout << "Test Case 3: " << solution.findInMountainArray(target3, ma3) << std::endl;

    // Test Case 4
    MountainArrayImpl ma4({1, 2, 5, 4, 3});
    int target4 = 5;
    std::cout << "Test Case 4: " << solution.findInMountainArray(target4, ma4) << std::endl;

    // Test Case 5
    MountainArrayImpl ma5({3, 5, 3, 2, 0});
    int target5 = 0;
    std::cout << "Test Case 5: " << solution.findInMountainArray(target5, ma5) << std::endl;

    return 0;
}