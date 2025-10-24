#include <iostream>
#include <vector>
#include <numeric>

class Task170 {
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
        return countAndMerge(prefixSums, 0, n, lower, upper);
    }

private:
    int countAndMerge(std::vector<long long>& sums, int start, int end, int lower, int upper) {
        if (start >= end) {
            return 0;
        }

        int mid = start + (end - start) / 2;
        int count = countAndMerge(sums, start, mid, lower, upper) + countAndMerge(sums, mid + 1, end, lower, upper);

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

        // Merge step
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
        
        for (int p = 0; p < temp.size(); ++p) {
            sums[start + p] = temp[p];
        }

        return count;
    }
};

void run_test_case(const std::string& name, std::vector<int>& nums, int lower, int upper) {
    Task170 solver;
    int result = solver.countRangeSum(nums, lower, upper);
    std::cout << name << ": " << result << std::endl;
}

int main() {
    // Test case 1
    std::vector<int> nums1 = {-2, 5, -1};
    run_test_case("Test Case 1", nums1, -2, 2);

    // Test case 2
    std::vector<int> nums2 = {0};
    run_test_case("Test Case 2", nums2, 0, 0);
    
    // Test case 3
    std::vector<int> nums3 = {2147483647, -2147483648, -1, 0};
    run_test_case("Test Case 3", nums3, -1, 0);

    // Test case 4
    std::vector<int> nums4 = {0, 0, 0};
    run_test_case("Test Case 4", nums4, 0, 0);

    // Test case 5
    std::vector<int> nums5 = {-1, 1};
    run_test_case("Test Case 5", nums5, 0, 0);
    
    return 0;
}