#include <iostream>
#include <vector>
#include <string>

class Task171 {
public:
    int reversePairs(std::vector<int>& nums) {
        if (nums.empty()) {
            return 0;
        }
        return mergeSortAndCount(nums, 0, nums.size() - 1);
    }

private:
    void merge(std::vector<int>& nums, int start, int mid, int end) {
        std::vector<int> temp;
        temp.reserve(end - start + 1);
        int i = start, j = mid + 1;
        
        while (i <= mid && j <= end) {
            if (nums[i] <= nums[j]) {
                temp.push_back(nums[i++]);
            } else {
                temp.push_back(nums[j++]);
            }
        }
        
        while (i <= mid) {
            temp.push_back(nums[i++]);
        }
        
        while (j <= end) {
            temp.push_back(nums[j++]);
        }
        
        for (int l = 0; l < temp.size(); ++l) {
            nums[start + l] = temp[l];
        }
    }

    int mergeSortAndCount(std::vector<int>& nums, int start, int end) {
        if (start >= end) {
            return 0;
        }

        int mid = start + (end - start) / 2;
        int count = mergeSortAndCount(nums, start, mid);
        count += mergeSortAndCount(nums, mid + 1, end);

        int j = mid + 1;
        for (int i = start; i <= mid; ++i) {
            while (j <= end && (long long)nums[i] > 2LL * nums[j]) {
                j++;
            }
            count += j - (mid + 1);
        }

        merge(nums, start, mid, end);

        return count;
    }
};

int main() {
    Task171 solver;

    // Test Case 1
    std::vector<int> nums1 = {1, 3, 2, 3, 1};
    std::cout << "Test Case 1: " << solver.reversePairs(nums1) << std::endl;

    // Test Case 2
    std::vector<int> nums2 = {2, 4, 3, 5, 1};
    std::cout << "Test Case 2: " << solver.reversePairs(nums2) << std::endl;

    // Test Case 3
    std::vector<int> nums3 = {2147483647, 2147483647, 2147483647, 2147483647, 2147483647};
    std::cout << "Test Case 3: " << solver.reversePairs(nums3) << std::endl;

    // Test Case 4
    std::vector<int> nums4 = {-5, -5};
    std::cout << "Test Case 4: " << solver.reversePairs(nums4) << std::endl;
    
    // Test Case 5
    std::vector<int> nums5 = {};
    std::cout << "Test Case 5: " << solver.reversePairs(nums5) << std::endl;

    return 0;
}