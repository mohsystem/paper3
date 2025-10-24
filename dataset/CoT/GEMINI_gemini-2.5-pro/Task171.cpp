#include <iostream>
#include <vector>
#include <algorithm>

class Task171 {
private:
    int merge(std::vector<int>& nums, int low, int mid, int high) {
        int count = 0;
        int j = mid + 1;
        for (int i = low; i <= mid; ++i) {
            while (j <= high && (long long)nums[i] > 2LL * nums[j]) {
                j++;
            }
            count += (j - (mid + 1));
        }

        std::vector<int> temp;
        int left = low, right = mid + 1;
        while (left <= mid && right <= high) {
            if (nums[left] <= nums[right]) {
                temp.push_back(nums[left++]);
            } else {
                temp.push_back(nums[right++]);
            }
        }
        while (left <= mid) {
            temp.push_back(nums[left++]);
        }
        while (right <= high) {
            temp.push_back(nums[right++]);
        }
        
        for(int i = 0; i < temp.size(); ++i) {
            nums[low + i] = temp[i];
        }

        return count;
    }

    int mergeSort(std::vector<int>& nums, int low, int high) {
        if (low >= high) {
            return 0;
        }
        int mid = low + (high - low) / 2;
        int count = mergeSort(nums, low, mid);
        count += mergeSort(nums, mid + 1, high);
        count += merge(nums, low, mid, high);
        return count;
    }

public:
    int reversePairs(std::vector<int>& nums) {
        if (nums.size() < 2) {
            return 0;
        }
        return mergeSort(nums, 0, nums.size() - 1);
    }
};

int main() {
    Task171 solution;

    // Test Case 1
    std::vector<int> nums1 = {1, 3, 2, 3, 1};
    std::cout << "Test Case 1: " << solution.reversePairs(nums1) << std::endl; // Expected: 2

    // Test Case 2
    std::vector<int> nums2 = {2, 4, 3, 5, 1};
    std::cout << "Test Case 2: " << solution.reversePairs(nums2) << std::endl; // Expected: 3

    // Test Case 3
    std::vector<int> nums3 = {5, 4, 3, 2, 1};
    std::cout << "Test Case 3: " << solution.reversePairs(nums3) << std::endl; // Expected: 4

    // Test Case 4
    std::vector<int> nums4 = {2147483647, 1000000000};
    std::cout << "Test Case 4: " << solution.reversePairs(nums4) << std::endl; // Expected: 1
    
    // Test Case 5
    std::vector<int> nums5 = {-1, -2, -3};
    std::cout << "Test Case 5: " << solution.reversePairs(nums5) << std::endl; // Expected: 3

    return 0;
}