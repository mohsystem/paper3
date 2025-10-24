#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <iomanip>

class Task185 {
public:
    double findMedianSortedArrays(const std::vector<int>& nums1, const std::vector<int>& nums2) {
        if (nums1.size() > nums2.size()) {
            return findMedianSortedArrays(nums2, nums1);
        }

        int m = nums1.size();
        int n = nums2.size();
        int low = 0;
        int high = m;
        
        int halfLen = (m + n + 1) / 2;

        while (low <= high) {
            int partitionX = low + (high - low) / 2;
            int partitionY = halfLen - partitionX;

            int maxLeftX = (partitionX == 0) ? std::numeric_limits<int>::min() : nums1[partitionX - 1];
            int minRightX = (partitionX == m) ? std::numeric_limits<int>::max() : nums1[partitionX];

            int maxLeftY = (partitionY == 0) ? std::numeric_limits<int>::min() : nums2[partitionY - 1];
            int minRightY = (partitionY == n) ? std::numeric_limits<int>::max() : nums2[partitionY];

            if (maxLeftX <= minRightY && maxLeftY <= minRightX) {
                if ((m + n) % 2 == 0) {
                    return (std::max(maxLeftX, maxLeftY) + std::min(minRightX, minRightY)) / 2.0;
                } else {
                    return static_cast<double>(std::max(maxLeftX, maxLeftY));
                }
            } else if (maxLeftX > minRightY) {
                high = partitionX - 1;
            } else {
                low = partitionX + 1;
            }
        }
        
        throw std::invalid_argument("Input arrays are not sorted or are invalid.");
    }
};

void run_test_case(Task185& sol, const std::string& name, const std::vector<int>& nums1, const std::vector<int>& nums2) {
    std::cout << name << ": " << std::fixed << std::setprecision(5) << sol.findMedianSortedArrays(nums1, nums2) << std::endl;
}

int main() {
    Task185 solution;

    // Test Case 1
    std::vector<int> nums1_1 = {1, 3};
    std::vector<int> nums2_1 = {2};
    run_test_case(solution, "Test Case 1", nums1_1, nums2_1);

    // Test Case 2
    std::vector<int> nums1_2 = {1, 2};
    std::vector<int> nums2_2 = {3, 4};
    run_test_case(solution, "Test Case 2", nums1_2, nums2_2);

    // Test Case 3
    std::vector<int> nums1_3 = {0, 0};
    std::vector<int> nums2_3 = {0, 0};
    run_test_case(solution, "Test Case 3", nums1_3, nums2_3);

    // Test Case 4
    std::vector<int> nums1_4 = {};
    std::vector<int> nums2_4 = {1};
    run_test_case(solution, "Test Case 4", nums1_4, nums2_4);

    // Test Case 5
    std::vector<int> nums1_5 = {2};
    std::vector<int> nums2_5 = {};
    run_test_case(solution, "Test Case 5", nums1_5, nums2_5);

    return 0;
}