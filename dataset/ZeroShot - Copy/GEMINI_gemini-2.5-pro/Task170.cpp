#include <iostream>
#include <vector>
#include <numeric>

class Task170 {
private:
    /**
     * A recursive helper function using a divide and conquer approach (Merge Sort).
     * It counts the valid range sums and sorts the prefix sum subarray.
     */
    int mergeSortAndCount(std::vector<long long>& sums, int start, int end, int lower, int upper) {
        if (start >= end) {
            return 0;
        }
        int mid = start + (end - start) / 2;
        int count = mergeSortAndCount(sums, start, mid, lower, upper) +
                    mergeSortAndCount(sums, mid + 1, end, lower, upper);

        // Count range sums S(i, j) where i is in the left half and j in the right half.
        // For each sums[j] in the right half, we need to find how many sums[i] in the left half
        // satisfy: sums[j] - upper <= sums[i] <= sums[j] - lower
        int k = start, l = start;
        for (int j = mid + 1; j <= end; ++j) {
            while (k <= mid && sums[k] < sums[j] - upper) {
                k++;
            }
            while (l <= mid && sums[l] <= sums[j] - lower) {
                l++;
            }
            count += l - k;
        }

        // Standard merge step for merge sort
        std::vector<long long> temp;
        temp.reserve(end - start + 1);
        int i = start, j = mid + 1;
        while (i <= mid && j <= end) {
            if (sums[i] <= sums[j]) {
                temp.push_back(sums[i++]);
            } else {
                temp.push_back(sums[j++]);
            }
        }
        while (i <= mid) {
            temp.push_back(sums[i++]);
        }
        while (j <= end) {
            temp.push_back(sums[j++]);
        }
        for (size_t p = 0; p < temp.size(); ++p) {
            sums[start + p] = temp[p];
        }
        
        return count;
    }

public:
    /**
     * Given an integer array nums and two integers lower and upper, return the number of range sums
     * that lie in [lower, upper] inclusive.
     */
    int countRangeSum(std::vector<int>& nums, int lower, int upper) {
        int n = nums.size();
        if (n == 0) return 0;
        // Prefix sums can exceed int range, so use long long.
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
    std::vector<int> nums3 = {1, 1, 1};
    int lower3 = 1, upper3 = 2;
    std::cout << "Test Case 3: " << solution.countRangeSum(nums3, lower3, upper3) << std::endl;

    // Test Case 4
    std::vector<int> nums4 = {-1, -1, -1};
    int lower4 = -2, upper4 = -1;
    std::cout << "Test Case 4: " << solution.countRangeSum(nums4, lower4, upper4) << std::endl;

    // Test Case 5
    std::vector<int> nums5 = {2147483647, -2147483647 - 1, -1, 0}; // INT_MAX, INT_MIN
    int lower5 = -1, upper5 = 0;
    std::cout << "Test Case 5: " << solution.countRangeSum(nums5, lower5, upper5) << std::endl;
    
    return 0;
}