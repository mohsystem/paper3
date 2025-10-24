
#include <iostream>
#include <vector>

// MountainArray interface (provided by the problem)
class MountainArray {
public:
    virtual int get(int index) = 0;
    virtual int length() = 0;
    virtual ~MountainArray() {}
};

class Task175 {
public:
    int findInMountainArray(int target, MountainArray &mountainArr) {
        // Input validation
        int length = mountainArr.length();
        if (length < 3) {
            return -1;
        }
        
        // Step 1: Find the peak index
        int peakIndex = findPeak(mountainArr, length);
        
        // Step 2: Search in the ascending part (left side)
        int result = binarySearchAscending(mountainArr, target, 0, peakIndex);
        
        // Step 3: If not found, search in the descending part (right side)
        if (result == -1) {
            result = binarySearchDescending(mountainArr, target, peakIndex + 1, length - 1);
        }
        
        return result;
    }
    
private:
    int findPeak(MountainArray &mountainArr, int length) {
        int left = 0;
        int right = length - 1;
        
        while (left < right) {
            int mid = left + (right - left) / 2;
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
    
    int binarySearchAscending(MountainArray &mountainArr, int target, int left, int right) {
        while (left <= right) {
            int mid = left + (right - left) / 2;
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
    
    int binarySearchDescending(MountainArray &mountainArr, int target, int left, int right) {
        while (left <= right) {
            int mid = left + (right - left) / 2;
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
};

// Test implementation of MountainArray
class TestMountainArray : public MountainArray {
private:
    std::vector<int> arr;
    
public:
    TestMountainArray(const std::vector<int>& arr) : arr(arr) {}
    
    int get(int index) override {
        return arr[index];
    }
    
    int length() override {
        return arr.size();
    }
};

int main() {
    Task175 solution;
    
    // Test case 1
    TestMountainArray test1({1, 2, 3, 4, 5, 3, 1});
    std::cout << "Test 1: " << solution.findInMountainArray(3, test1) << std::endl; // Expected: 2
    
    // Test case 2
    TestMountainArray test2({0, 1, 2, 4, 2, 1});
    std::cout << "Test 2: " << solution.findInMountainArray(3, test2) << std::endl; // Expected: -1
    
    // Test case 3
    TestMountainArray test3({1, 2, 3, 4, 5, 3, 1});
    std::cout << "Test 3: " << solution.findInMountainArray(5, test3) << std::endl; // Expected: 4
    
    // Test case 4
    TestMountainArray test4({0, 5, 3, 1});
    std::cout << "Test 4: " << solution.findInMountainArray(1, test4) << std::endl; // Expected: 3
    
    // Test case 5
    TestMountainArray test5({1, 5, 2});
    std::cout << "Test 5: " << solution.findInMountainArray(2, test5) << std::endl; // Expected: 2
    
    return 0;
}
