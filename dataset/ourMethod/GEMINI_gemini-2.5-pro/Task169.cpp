#include <iostream>
#include <vector>
#include <utility>

class Solution {
private:
    std::vector<int> counts;

    void merge(std::vector<std::pair<int, int>>& items, int start, int mid, int end) {
        std::vector<std::pair<int, int>> temp;
        temp.reserve(end - start + 1);

        int i = start;
        int j = mid + 1;

        while (i <= mid && j <= end) {
            if (items[i].first <= items[j].first) {
                counts[items[i].second] += j - (mid + 1);
                temp.push_back(items[i]);
                i++;
            } else {
                temp.push_back(items[j]);
                j++;
            }
        }

        while (i <= mid) {
            counts[items[i].second] += j - (mid + 1);
            temp.push_back(items[i]);
            i++;
        }

        while (j <= end) {
            temp.push_back(items[j]);
            j++;
        }

        for (size_t k = 0; k < temp.size(); ++k) {
            items[start + k] = temp[k];
        }
    }

    void mergeSort(std::vector<std::pair<int, int>>& items, int start, int end) {
        if (start >= end) {
            return;
        }
        int mid = start + (end - start) / 2;
        mergeSort(items, start, mid);
        mergeSort(items, mid + 1, end);
        merge(items, start, mid, end);
    }

public:
    std::vector<int> countSmaller(const std::vector<int>& nums) {
        int n = nums.size();
        if (n == 0) {
            return {};
        }

        counts.assign(n, 0);
        std::vector<std::pair<int, int>> items(n);
        for (int i = 0; i < n; ++i) {
            items[i] = {nums[i], i};
        }

        mergeSort(items, 0, n - 1);

        return counts;
    }
};

void print_vector(const std::string& prefix, const std::vector<int>& vec) {
    std::cout << prefix;
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i];
        if (i < vec.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;
}

int main() {
    Solution solution;
    
    // Test Case 1
    std::vector<int> nums1 = {5, 2, 6, 1};
    std::cout << "Test Case 1:" << std::endl;
    print_vector("Input: ", nums1);
    print_vector("Output: ", solution.countSmaller(nums1)); // Expected: [2, 1, 1, 0]

    // Test Case 2
    std::vector<int> nums2 = {-1};
    std::cout << "\nTest Case 2:" << std::endl;
    print_vector("Input: ", nums2);
    print_vector("Output: ", solution.countSmaller(nums2)); // Expected: [0]

    // Test Case 3
    std::vector<int> nums3 = {-1, -1};
    std::cout << "\nTest Case 3:" << std::endl;
    print_vector("Input: ", nums3);
    print_vector("Output: ", solution.countSmaller(nums3)); // Expected: [0, 0]

    // Test Case 4
    std::vector<int> nums4 = {};
    std::cout << "\nTest Case 4:" << std::endl;
    print_vector("Input: ", nums4);
    print_vector("Output: ", solution.countSmaller(nums4)); // Expected: []

    // Test Case 5
    std::vector<int> nums5 = {2, 0, 1};
    std::cout << "\nTest Case 5:" << std::endl;
    print_vector("Input: ", nums5);
    print_vector("Output: ", solution.countSmaller(nums5)); // Expected: [2, 0, 0]

    return 0;
}