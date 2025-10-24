#include <iostream>
#include <vector>
#include <utility>

class Task169 {
private:
    void merge(std::vector<std::pair<int, int>>& items, int left, int mid, int right, std::vector<int>& counts) {
        std::vector<std::pair<int, int>> leftArr(items.begin() + left, items.begin() + mid + 1);
        std::vector<std::pair<int, int>> rightArr(items.begin() + mid + 1, items.begin() + right + 1);

        int i = 0, j = 0, k = left;
        while (i < leftArr.size() && j < rightArr.size()) {
            if (leftArr[i].first <= rightArr[j].first) {
                counts[leftArr[i].second] += j;
                items[k++] = leftArr[i++];
            } else {
                items[k++] = rightArr[j++];
            }
        }
        while (i < leftArr.size()) {
            counts[leftArr[i].second] += j;
            items[k++] = leftArr[i++];
        }
        while (j < rightArr.size()) {
            items[k++] = rightArr[j++];
        }
    }

    void mergeSort(std::vector<std::pair<int, int>>& items, int left, int right, std::vector<int>& counts) {
        if (left >= right) {
            return;
        }
        int mid = left + (right - left) / 2;
        mergeSort(items, left, mid, counts);
        mergeSort(items, mid + 1, right, counts);
        merge(items, left, mid, right, counts);
    }

public:
    std::vector<int> countSmaller(std::vector<int>& nums) {
        int n = nums.size();
        if (n == 0) {
            return {};
        }

        std::vector<std::pair<int, int>> items(n);
        for (int i = 0; i < n; ++i) {
            items[i] = {nums[i], i};
        }

        std::vector<int> counts(n, 0);
        mergeSort(items, 0, n - 1, counts);
        return counts;
    }
};

void printVector(const std::vector<int>& vec) {
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
    std::cout << "Test Case 1 Output: ";
    printVector(solution.countSmaller(nums1));

    // Test Case 2
    std::vector<int> nums2 = {-1};
    std::cout << "Test Case 2 Output: ";
    printVector(solution.countSmaller(nums2));

    // Test Case 3
    std::vector<int> nums3 = {-1, -1};
    std::cout << "Test Case 3 Output: ";
    printVector(solution.countSmaller(nums3));
    
    // Test Case 4
    std::vector<int> nums4 = {5, 4, 3, 2, 1};
    std::cout << "Test Case 4 Output: ";
    printVector(solution.countSmaller(nums4));

    // Test Case 5
    std::vector<int> nums5 = {};
    std::cout << "Test Case 5 Output: ";
    printVector(solution.countSmaller(nums5));

    return 0;
}