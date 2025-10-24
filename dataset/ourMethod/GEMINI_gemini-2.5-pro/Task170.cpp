#include <iostream>
#include <vector>
#include <numeric>

int mergeSortAndCount(std::vector<long long>& sums, int start, int end, int lower, int upper) {
    if (end - start <= 1) {
        return 0;
    }

    int mid = start + (end - start) / 2;
    int count = mergeSortAndCount(sums, start, mid, lower, upper)
              + mergeSortAndCount(sums, mid, end, lower, upper);

    int l = mid, r = mid;
    for (int i = start; i < mid; ++i) {
        while (l < end && sums[l] - sums[i] < lower) {
            l++;
        }
        while (r < end && sums[r] - sums[i] <= upper) {
            r++;
        }
        count += r - l;
    }

    std::vector<long long> temp;
    temp.reserve(end - start);
    int i = start, j = mid;
    while(i < mid && j < end) {
        if(sums[i] <= sums[j]) {
            temp.push_back(sums[i++]);
        } else {
            temp.push_back(sums[j++]);
        }
    }
    while(i < mid) {
        temp.push_back(sums[i++]);
    }
    while(j < end) {
        temp.push_back(sums[j++]);
    }

    for(size_t k=0; k < temp.size(); ++k) {
        sums[start + k] = temp[k];
    }

    return count;
}

int countRangeSum(std::vector<int>& nums, int lower, int upper) {
    int n = nums.size();
    std::vector<long long> prefixSums(n + 1, 0);
    for (int i = 0; i < n; ++i) {
        prefixSums[i + 1] = prefixSums[i] + nums[i];
    }

    return mergeSortAndCount(prefixSums, 0, n + 1, lower, upper);
}

int main() {
    // Test case 1
    std::vector<int> nums1 = {-2, 5, -1};
    int lower1 = -2, upper1 = 2;
    std::cout << "Test 1: " << countRangeSum(nums1, lower1, upper1) << std::endl;

    // Test case 2
    std::vector<int> nums2 = {0};
    int lower2 = 0, upper2 = 0;
    std::cout << "Test 2: " << countRangeSum(nums2, lower2, upper2) << std::endl;

    // Test case 3
    std::vector<int> nums3 = {2147483647, -2147483647, -1, 0};
    int lower3 = -1, upper3 = 0;
    std::cout << "Test 3: " << countRangeSum(nums3, lower3, upper3) << std::endl;

    // Test case 4
    std::vector<int> nums4 = {0, 0, 0};
    int lower4 = 0, upper4 = 0;
    std::cout << "Test 4: " << countRangeSum(nums4, lower4, upper4) << std::endl;
    
    // Test case 5
    std::vector<int> nums5 = {-1, 1};
    int lower5 = 0, upper5 = 0;
    std::cout << "Test 5: " << countRangeSum(nums5, lower5, upper5) << std::endl;

    return 0;
}