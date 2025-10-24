#include <iostream>
#include <vector>
#include <string>

class Solution {
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
        temp.reserve(high - low + 1);
        int i = low, k = mid + 1;
        while (i <= mid && k <= high) {
            if (nums[i] <= nums[k]) {
                temp.push_back(nums[i++]);
            } else {
                temp.push_back(nums[k++]);
            }
        }
        while (i <= mid) {
            temp.push_back(nums[i++]);
        }
        while (k <= high) {
            temp.push_back(nums[k++]);
        }

        for (size_t l = 0; l < temp.size(); ++l) {
            nums[low + l] = temp[l];
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
        if (nums.empty()) {
            return 0;
        }
        return mergeSort(nums, 0, nums.size() - 1);
    }
};

void run_test_case(const std::string& name, std::vector<int> nums) {
    Solution sol;
    std::cout << name << ": " << sol.reversePairs(nums) << std::endl;
}

int main() {
    run_test_case("Test Case 1", {1, 3, 2, 3, 1});
    run_test_case("Test Case 2", {2, 4, 3, 5, 1});
    run_test_case("Test Case 3", {2147483647, 2147483647, 2147483647, 2147483647, 2147483647, 2147483647});
    run_test_case("Test Case 4", {});
    run_test_case("Test Case 5", {5, 4, 3, 2, 1});
    return 0;
}