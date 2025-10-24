#include <iostream>
#include <vector>
#include <deque>

/**
 * @brief Finds the maximum value in a sliding window of size k.
 * 
 * @param nums The input vector of integers.
 * @param k The size of the sliding window.
 * @return A vector containing the maximum of each sliding window.
 */
std::vector<int> maxSlidingWindow(const std::vector<int>& nums, int k) {
    if (nums.empty() || k <= 0 || k > nums.size()) {
        return {};
    }

    std::vector<int> result;
    // Deque stores indices of elements from nums.
    std::deque<int> dq;
    int n = nums.size();

    for (int i = 0; i < n; ++i) {
        // 1. Remove indices from the front that are out of the current window.
        if (!dq.empty() && dq.front() <= i - k) {
            dq.pop_front();
        }

        // 2. Remove indices from the back whose corresponding elements are smaller
        //    than the current element.
        while (!dq.empty() && nums[dq.back()] < nums[i]) {
            dq.pop_back();
        }

        // 3. Add the current index to the back.
        dq.push_back(i);

        // 4. Once the window is full, the maximum is at the front of the deque.
        if (i >= k - 1) {
            result.push_back(nums[dq.front()]);
        }
    }

    return result;
}

void print_vector(const std::string& prefix, const std::vector<int>& vec) {
    std::cout << prefix;
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
    std::vector<int> output1 = maxSlidingWindow(nums1, k1);
    print_vector("Test Case 1: ", output1); // Expected: [3, 3, 5, 5, 6, 7]

    // Test Case 2
    std::vector<int> nums2 = {1};
    int k2 = 1;
    std::vector<int> output2 = maxSlidingWindow(nums2, k2);
    print_vector("Test Case 2: ", output2); // Expected: [1]

    // Test Case 3
    std::vector<int> nums3 = {1, -1};
    int k3 = 1;
    std::vector<int> output3 = maxSlidingWindow(nums3, k3);
    print_vector("Test Case 3: ", output3); // Expected: [1, -1]

    // Test Case 4
    std::vector<int> nums4 = {9, 11};
    int k4 = 2;
    std::vector<int> output4 = maxSlidingWindow(nums4, k4);
    print_vector("Test Case 4: ", output4); // Expected: [11]

    // Test Case 5
    std::vector<int> nums5 = {7, 2, 4};
    int k5 = 2;
    std::vector<int> output5 = maxSlidingWindow(nums5, k5);
    print_vector("Test Case 5: ", output5); // Expected: [7, 4]

    return 0;
}