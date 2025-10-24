#include <iostream>
#include <vector>
#include <deque>
#include <string>

/**
 * Finds the maximum value in a sliding window of size k.
 *
 * @param nums The input vector of integers.
 * @param k    The size of the sliding window.
 * @return A vector containing the maximum of each sliding window.
 */
std::vector<int> maxSlidingWindow(const std::vector<int>& nums, int k) {
    if (nums.empty() || k <= 0 || k > nums.size()) {
        return {};
    }

    int n = nums.size();
    std::vector<int> result;
    result.reserve(n - k + 1);
    
    // Deque stores indices of elements in the current window.
    // The indices point to values in decreasing order.
    std::deque<int> dq;

    for (int i = 0; i < n; ++i) {
        // Remove indices from the front of the deque that are out of the current window.
        if (!dq.empty() && dq.front() <= i - k) {
            dq.pop_front();
        }

        // Maintain the decreasing order of values in the deque.
        // Remove indices from the back whose corresponding values are smaller than the current element.
        while (!dq.empty() && nums[dq.back()] < nums[i]) {
            dq.pop_back();
        }

        // Add the current index to the back of the deque.
        dq.push_back(i);

        // Once the window is full (i.e., we have processed at least k elements),
        // the maximum for the current window is at the front of the deque.
        if (i >= k - 1) {
            result.push_back(nums[dq.front()]);
        }
    }
    return result;
}

void printVector(const std::vector<int>& vec) {
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << (i == vec.size() - 1 ? "" : ", ");
    }
    std::cout << "]";
}

int main() {
    // Test Case 1
    std::vector<int> nums1 = {1, 3, -1, -3, 5, 3, 6, 7};
    int k1 = 3;
    std::cout << "Test Case 1:" << std::endl;
    std::cout << "Input: nums = "; printVector(nums1); std::cout << ", k = " << k1 << std::endl;
    std::vector<int> res1 = maxSlidingWindow(nums1, k1);
    std::cout << "Output: "; printVector(res1); std::cout << std::endl << std::endl;

    // Test Case 2
    std::vector<int> nums2 = {1};
    int k2 = 1;
    std::cout << "Test Case 2:" << std::endl;
    std::cout << "Input: nums = "; printVector(nums2); std::cout << ", k = " << k2 << std::endl;
    std::vector<int> res2 = maxSlidingWindow(nums2, k2);
    std::cout << "Output: "; printVector(res2); std::cout << std::endl << std::endl;

    // Test Case 3
    std::vector<int> nums3 = {1, -1};
    int k3 = 1;
    std::cout << "Test Case 3:" << std::endl;
    std::cout << "Input: nums = "; printVector(nums3); std::cout << ", k = " << k3 << std::endl;
    std::vector<int> res3 = maxSlidingWindow(nums3, k3);
    std::cout << "Output: "; printVector(res3); std::cout << std::endl << std::endl;

    // Test Case 4
    std::vector<int> nums4 = {7, 2, 4};
    int k4 = 2;
    std::cout << "Test Case 4:" << std::endl;
    std::cout << "Input: nums = "; printVector(nums4); std::cout << ", k = " << k4 << std::endl;
    std::vector<int> res4 = maxSlidingWindow(nums4, k4);
    std::cout << "Output: "; printVector(res4); std::cout << std::endl << std::endl;

    // Test Case 5
    std::vector<int> nums5 = {1, 3, 1, 2, 0, 5};
    int k5 = 3;
    std::cout << "Test Case 5:" << std::endl;
    std::cout << "Input: nums = "; printVector(nums5); std::cout << ", k = " << k5 << std::endl;
    std::vector<int> res5 = maxSlidingWindow(nums5, k5);
    std::cout << "Output: "; printVector(res5); std::cout << std::endl << std::endl;

    return 0;
}