#include <iostream>
#include <vector>
#include <utility>

class Task169 {
public:
    std::vector<int> countSmaller(std::vector<int>& nums) {
        int n = nums.size();
        if (n == 0) {
            return {};
        }

        std::vector<std::pair<int, int>> pairs(n);
        for (int i = 0; i < n; ++i) {
            pairs[i] = {nums[i], i};
        }

        std::vector<int> counts(n, 0);
        mergeSort(pairs, 0, n - 1, counts);
        return counts;
    }

private:
    void mergeSort(std::vector<std::pair<int, int>>& pairs, int start, int end, std::vector<int>& counts) {
        if (start >= end) {
            return;
        }

        int mid = start + (end - start) / 2;
        mergeSort(pairs, start, mid, counts);
        mergeSort(pairs, mid + 1, end, counts);
        merge(pairs, start, mid, end, counts);
    }

    void merge(std::vector<std::pair<int, int>>& pairs, int start, int mid, int end, std::vector<int>& counts) {
        std::vector<std::pair<int, int>> temp;
        temp.reserve(end - start + 1);

        int i = start;
        int j = mid + 1;
        int rightElementsSmaller = 0;

        while (i <= mid && j <= end) {
            if (pairs[i].first > pairs[j].first) {
                temp.push_back(pairs[j++]);
                rightElementsSmaller++;
            } else {
                counts[pairs[i].second] += rightElementsSmaller;
                temp.push_back(pairs[i++]);
            }
        }

        while (i <= mid) {
            counts[pairs[i].second] += rightElementsSmaller;
            temp.push_back(pairs[i++]);
        }

        while (j <= end) {
            temp.push_back(pairs[j++]);
        }
        
        for (int k = 0; k < temp.size(); ++k) {
            pairs[start + k] = temp[k];
        }
    }
};

void print_vector(const std::vector<int>& vec) {
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << (i == vec.size() - 1 ? "" : ", ");
    }
    std::cout << "]" << std::endl;
}

int main() {
    Task169 solution;

    // Test Case 1
    std::vector<int> nums1 = {5, 2, 6, 1};
    std::vector<int> result1 = solution.countSmaller(nums1);
    std::cout << "Test Case 1: ";
    print_vector(result1);

    // Test Case 2
    std::vector<int> nums2 = {-1};
    std::vector<int> result2 = solution.countSmaller(nums2);
    std::cout << "Test Case 2: ";
    print_vector(result2);

    // Test Case 3
    std::vector<int> nums3 = {-1, -1};
    std::vector<int> result3 = solution.countSmaller(nums3);
    std::cout << "Test Case 3: ";
    print_vector(result3);

    // Test Case 4
    std::vector<int> nums4 = {};
    std::vector<int> result4 = solution.countSmaller(nums4);
    std::cout << "Test Case 4: ";
    print_vector(result4);

    // Test Case 5
    std::vector<int> nums5 = {2, 0, 1};
    std::vector<int> result5 = solution.countSmaller(nums5);
    std::cout << "Test Case 5: ";
    print_vector(result5);

    return 0;
}