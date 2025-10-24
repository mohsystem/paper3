#include <iostream>
#include <vector>
#include <utility>

class Task169 {
private:
    void merge(std::vector<std::pair<int, int>>& items, int start, int mid, int end, std::vector<int>& counts) {
        std::vector<std::pair<int, int>> temp;
        temp.reserve(end - start + 1);

        int i = start;
        int j = mid + 1;
        int rightCounter = 0;

        while (i <= mid && j <= end) {
            if (items[i].first <= items[j].first) {
                counts[items[i].second] += rightCounter;
                temp.push_back(items[i++]);
            } else {
                rightCounter++;
                temp.push_back(items[j++]);
            }
        }

        while (i <= mid) {
            counts[items[i].second] += rightCounter;
            temp.push_back(items[i++]);
        }

        while (j <= end) {
            temp.push_back(items[j++]);
        }

        for (int k = 0; k < temp.size(); ++k) {
            items[start + k] = temp[k];
        }
    }

    void mergeSort(std::vector<std::pair<int, int>>& items, int start, int end, std::vector<int>& counts) {
        if (start >= end) {
            return;
        }
        int mid = start + (end - start) / 2;
        mergeSort(items, start, mid, counts);
        mergeSort(items, mid + 1, end, counts);
        merge(items, start, mid, end, counts);
    }

public:
    std::vector<int> countSmaller(const std::vector<int>& nums) {
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
    std::cout << "]";
}

int main() {
    Task169 solver;

    // Test Case 1
    std::vector<int> nums1 = {5, 2, 6, 1};
    std::cout << "Test Case 1:\nInput: ";
    printVector(nums1);
    std::vector<int> result1 = solver.countSmaller(nums1);
    std::cout << "\nOutput: ";
    printVector(result1);
    std::cout << std::endl;

    // Test Case 2
    std::vector<int> nums2 = {-1};
    std::cout << "\nTest Case 2:\nInput: ";
    printVector(nums2);
    std::vector<int> result2 = solver.countSmaller(nums2);
    std::cout << "\nOutput: ";
    printVector(result2);
    std::cout << std::endl;

    // Test Case 3
    std::vector<int> nums3 = {-1, -1};
    std::cout << "\nTest Case 3:\nInput: ";
    printVector(nums3);
    std::vector<int> result3 = solver.countSmaller(nums3);
    std::cout << "\nOutput: ";
    printVector(result3);
    std::cout << std::endl;

    // Test Case 4
    std::vector<int> nums4 = {2, 0, 1};
    std::cout << "\nTest Case 4:\nInput: ";
    printVector(nums4);
    std::vector<int> result4 = solver.countSmaller(nums4);
    std::cout << "\nOutput: ";
    printVector(result4);
    std::cout << std::endl;

    // Test Case 5
    std::vector<int> nums5 = {};
    std::cout << "\nTest Case 5:\nInput: ";
    printVector(nums5);
    std::vector<int> result5 = solver.countSmaller(nums5);
    std::cout << "\nOutput: ";
    printVector(result5);
    std::cout << std::endl;

    return 0;
}