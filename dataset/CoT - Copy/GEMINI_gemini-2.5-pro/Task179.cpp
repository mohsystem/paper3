#include <iostream>
#include <vector>
#include <deque>

class Task179 {
public:
    /**
     * Finds the maximum value in a sliding window of size k.
     *
     * @param nums The input vector of integers.
     * @param k The size of the sliding window.
     * @return A vector containing the maximum of each sliding window.
     */
    static std::vector<int> maxSlidingWindow(const std::vector<int>& nums, int k) {
        if (nums.empty() || k <= 0 || k > nums.size()) {
            return {};
        }

        int n = nums.size();
        std::vector<int> result;
        result.reserve(n - k + 1);

        // Deque stores indices of elements in the current window.
        // The elements corresponding to these indices are in decreasing order.
        std::deque<int> dq;

        for (int i = 0; i < n; ++i) {
            // Remove indices from the front of the deque that are out of the current window.
            if (!dq.empty() && dq.front() <= i - k) {
                dq.pop_front();
            }

            // Maintain the decreasing order property of the deque.
            // Remove indices from the back whose corresponding elements are smaller than or equal to the current element.
            while (!dq.empty() && nums[dq.back()] <= nums[i]) {
                dq.pop_back();
            }

            // Add the current element's index to the back of the deque.
            dq.push_back(i);

            // Once the window is fully formed, the maximum element is at the front of the deque.
            if (i >= k - 1) {
                result.push_back(nums[dq.front()]);
            }
        }
        return result;
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
    // Test Case 1
    std::vector<int> nums1 = {1, 3, -1, -3, 5, 3, 6, 7};
    int k1 = 3;
    std::cout << "Test Case 1: nums = [1, 3, -1, -3, 5, 3, 6, 7], k = 3" << std::endl;
    std::vector<int> result1 = Task179::maxSlidingWindow(nums1, k1);
    std::cout << "Output: ";
    printVector(result1); // Expected: [3, 3, 5, 5, 6, 7]

    // Test Case 2
    std::vector<int> nums2 = {1};
    int k2 = 1;
    std::cout << "\nTest Case 2: nums = [1], k = 1" << std::endl;
    std::vector<int> result2 = Task179::maxSlidingWindow(nums2, k2);
    std::cout << "Output: ";
    printVector(result2); // Expected: [1]

    // Test Case 3
    std::vector<int> nums3 = {1, -1};
    int k3 = 1;
    std::cout << "\nTest Case 3: nums = [1, -1], k = 1" << std::endl;
    std::vector<int> result3 = Task179::maxSlidingWindow(nums3, k3);
    std::cout << "Output: ";
    printVector(result3); // Expected: [1, -1]
    
    // Test Case 4
    std::vector<int> nums4 = {9, 9, 9, 9};
    int k4 = 2;
    std::cout << "\nTest Case 4: nums = [9, 9, 9, 9], k = 2" << std::endl;
    std::vector<int> result4 = Task179::maxSlidingWindow(nums4, k4);
    std::cout << "Output: ";
    printVector(result4); // Expected: [9, 9, 9]

    // Test Case 5
    std::vector<int> nums5 = {7, 6, 5, 4, 3, 2, 1};
    int k5 = 4;
    std::cout << "\nTest Case 5: nums = [7, 6, 5, 4, 3, 2, 1], k = 4" << std::endl;
    std::vector<int> result5 = Task179::maxSlidingWindow(nums5, k5);
    std::cout << "Output: ";
    printVector(result5); // Expected: [7, 6, 5, 4]

    return 0;
}