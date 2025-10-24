#include <iostream>
#include <vector>
#include <numeric>
#include <climits>

class Task170 {
private:
    int mergeSortAndCount(std::vector<long long>& sums, int start, int end, int lower, int upper) {
        if (start >= end) {
            return 0;
        }

        int mid = start + (end - start) / 2;
        int count = mergeSortAndCount(sums, start, mid, lower, upper) + 
                    mergeSortAndCount(sums, mid + 1, end, lower, upper);

        int l = mid + 1;
        int r = mid + 1;
        for (int i = start; i <= mid; ++i) {
            while (l <= end && sums[l] - sums[i] < lower) {
                l++;
            }
            while (r <= end && sums[r] - sums[i] <= upper) {
                r++;
            }
            count += (r - l);
        }

        // Merge step
        std::vector<long long> temp;
        temp.reserve(end - start + 1);
        int p1 = start, p2 = mid + 1;
        while (p1 <= mid && p2 <= end) {
            if (sums[p1] <= sums[p2]) {
                temp.push_back(sums[p1++]);
            } else {
                temp.push_back(sums[p2++]);
            }
        }
        while (p1 <= mid) {
            temp.push_back(sums[p1++]);
        }
        while (p2 <= end) {
            temp.push_back(sums[p2++]);
        }

        for (size_t i = 0; i < temp.size(); ++i) {
            sums[start + i] = temp[i];
        }

        return count;
    }

public:
    int countRangeSum(std::vector<int>& nums, int lower, int upper) {
        if (nums.empty()) {
            return 0;
        }
        int n = nums.size();
        std::vector<long long> prefixSums(n + 1, 0);
        for (int i = 0; i < n; ++i) {
            prefixSums[i + 1] = prefixSums[i] + nums[i];
        }
        return mergeSortAndCount(prefixSums, 0, n, lower, upper);
    }
};

int main() {
    Task170 solution;

    // Test Case 1
    std::vector<int> nums1 = {-2, 5, -1};
    int lower1 = -2, upper1 = 2;
    std::cout << "Test Case 1: " << solution.countRangeSum(nums1, lower1, upper1) << std::endl;

    // Test Case 2
    std::vector<int> nums2 = {0};
    int lower2 = 0, upper2 = 0;
    std::cout << "Test Case 2: " << solution.countRangeSum(nums2, lower2, upper2) << std::endl;

    // Test Case 3
    std::vector<int> nums3 = {1, -1, 1, -1};
    int lower3 = 0, upper3 = 0;
    std::cout << "Test Case 3: " << solution.countRangeSum(nums3, lower3, upper3) << std::endl;

    // Test Case 4
    std::vector<int> nums4 = {INT_MAX, INT_MIN, 0, -1, 1};
    int lower4 = -1, upper4 = 1;
    std::cout << "Test Case 4: " << solution.countRangeSum(nums4, lower4, upper4) << std::endl;
    
    // Test Case 5
    std::vector<int> nums5 = {0, 0, 0, 0, 0};
    int lower5 = 0, upper5 = 0;
    std::cout << "Test Case 5: " << solution.countRangeSum(nums5, lower5, upper5) << std::endl;
    
    return 0;
}