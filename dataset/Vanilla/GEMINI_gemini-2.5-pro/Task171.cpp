#include <iostream>
#include <vector>
#include <algorithm>

class Task171 {
public:
    int reversePairs(std::vector<int>& nums) {
        if (nums.empty()) {
            return 0;
        }
        return mergeSort(nums, 0, nums.size() - 1);
    }

private:
    int mergeSort(std::vector<int>& nums, int left, int right) {
        if (left >= right) {
            return 0;
        }
        int mid = left + (right - left) / 2;
        int count = mergeSort(nums, left, mid);
        count += mergeSort(nums, mid + 1, right);
        count += mergeAndCount(nums, left, mid, right);
        return count;
    }

    int mergeAndCount(std::vector<int>& nums, int left, int mid, int right) {
        int count = 0;
        int j = mid + 1;
        for (int i = left; i <= mid; ++i) {
            while (j <= right && (long long)nums[i] > 2LL * nums[j]) {
                j++;
            }
            count += (j - (mid + 1));
        }

        std::vector<int> temp;
        temp.reserve(right - left + 1);
        int i = left, k = mid + 1;
        while (i <= mid && k <= right) {
            if (nums[i] <= nums[k]) {
                temp.push_back(nums[i++]);
            } else {
                temp.push_back(nums[k++]);
            }
        }
        while (i <= mid) {
            temp.push_back(nums[i++]);
        }
        while (k <= right) {
            temp.push_back(nums[k++]);
        }
        for (size_t l = 0; l < temp.size(); ++l) {
            nums[left + l] = temp[l];
        }
        
        return count;
    }
};

int main() {
    Task171 solution;

    // Test Case 1
    std::vector<int> nums1 = {1, 3, 2, 3, 1};
    std::cout << "Test Case 1: " << solution.reversePairs(nums1) << std::endl;

    // Test Case 2
    std::vector<int> nums2 = {2, 4, 3, 5, 1};
    std::cout << "Test Case 2: " << solution.reversePairs(nums2) << std::endl;

    // Test Case 3
    std::vector<int> nums3 = {};
    std::cout << "Test Case 3: " << solution.reversePairs(nums3) << std::endl;

    // Test Case 4
    std::vector<int> nums4 = {5, 4, 3, 2, 1};
    std::cout << "Test Case 4: " << solution.reversePairs(nums4) << std::endl;

    // Test Case 5
    std::vector<int> nums5 = {2147483647, 2147483647, 2147483647, 2147483647, 2147483647, 2147483647};
    std::cout << "Test Case 5: " << solution.reversePairs(nums5) << std::endl;

    return 0;
}