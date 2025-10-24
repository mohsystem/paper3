#include <iostream>
#include <vector>
#include <deque>
#include <string>

class Task179 {
public:
    std::vector<int> maxSlidingWindow(const std::vector<int>& nums, int k) {
        // Handle edge cases based on constraints
        if (nums.empty() || k <= 0) {
            return {};
        }
        
        int n = nums.size();
        std::vector<int> result;
        // Pre-allocate memory for efficiency
        result.reserve(n - k + 1); 
        
        // Deque stores indices. The values corresponding to these indices are in decreasing order.
        std::deque<int> dq; 

        for (int i = 0; i < n; ++i) {
            // 1. Remove indices from the front that are out of the current window's scope
            if (!dq.empty() && dq.front() <= i - k) {
                dq.pop_front();
            }

            // 2. Maintain the decreasing order of values in the deque.
            while (!dq.empty() && nums[dq.back()] < nums[i]) {
                dq.pop_back();
            }

            // 3. Add the current element's index to the deque
            dq.push_back(i);

            // 4. If the window has at least k elements, the front of the deque is the max.
            if (i >= k - 1) {
                result.push_back(nums[dq.front()]);
            }
        }

        return result;
    }
};

void printVector(const std::string& prefix, const std::vector<int>& vec) {
    std::cout << prefix << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << (i == vec.size() - 1 ? "" : ", ");
    }
    std::cout << "]";
}

int main() {
    Task179 solution;

    // Test Case 1
    std::vector<int> nums1 = {1, 3, -1, -3, 5, 3, 6, 7};
    int k1 = 3;
    std::cout << "Test Case 1:" << std::endl;
    printVector("Input: nums = ", nums1);
    std::cout << ", k = " << k1 << std::endl;
    std::vector<int> result1 = solution.maxSlidingWindow(nums1, k1);
    printVector("Output: ", result1);
    std::cout << std::endl;
    printVector("Expected: ", {3, 3, 5, 5, 6, 7});
    std::cout << "\n\n";

    // Test Case 2
    std::vector<int> nums2 = {1};
    int k2 = 1;
    std::cout << "Test Case 2:" << std::endl;
    printVector("Input: nums = ", nums2);
    std::cout << ", k = " << k2 << std::endl;
    std::vector<int> result2 = solution.maxSlidingWindow(nums2, k2);
    printVector("Output: ", result2);
    std::cout << std::endl;
    printVector("Expected: ", {1});
    std::cout << "\n\n";

    // Test Case 3
    std::vector<int> nums3 = {1, -1};
    int k3 = 1;
    std::cout << "Test Case 3:" << std::endl;
    printVector("Input: nums = ", nums3);
    std::cout << ", k = " << k3 << std::endl;
    std::vector<int> result3 = solution.maxSlidingWindow(nums3, k3);
    printVector("Output: ", result3);
    std::cout << std::endl;
    printVector("Expected: ", {1, -1});
    std::cout << "\n\n";
    
    // Test Case 4
    std::vector<int> nums4 = {9, 10, 9, -7, -4, -8, 2, -6};
    int k4 = 5;
    std::cout << "Test Case 4:" << std::endl;
    printVector("Input: nums = ", nums4);
    std::cout << ", k = " << k4 << std::endl;
    std::vector<int> result4 = solution.maxSlidingWindow(nums4, k4);
    printVector("Output: ", result4);
    std::cout << std::endl;
    printVector("Expected: ", {10, 10, 9, 2, 2});
    std::cout << "\n\n";

    // Test Case 5
    std::vector<int> nums5 = {7, 2, 4};
    int k5 = 2;
    std::cout << "Test Case 5:" << std::endl;
    printVector("Input: nums = ", nums5);
    std::cout << ", k = " << k5 << std::endl;
    std::vector<int> result5 = solution.maxSlidingWindow(nums5, k5);
    printVector("Output: ", result5);
    std::cout << std::endl;
    printVector("Expected: ", {7, 4});
    std::cout << "\n\n";

    return 0;
}