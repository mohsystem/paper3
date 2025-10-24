
#include <iostream>
#include <vector>
#include <deque>
#include <climits>
using namespace std;

class Task178 {
public:
    static int shortestSubarray(vector<int>& nums, int k) {
        int n = nums.size();
        vector<long long> prefixSum(n + 1, 0);
        
        // Calculate prefix sums
        for (int i = 0; i < n; i++) {
            prefixSum[i + 1] = prefixSum[i] + nums[i];
        }
        
        int minLength = INT_MAX;
        deque<int> dq;
        
        for (int i = 0; i <= n; i++) {
            // Check if we can form a valid subarray ending at i
            while (!dq.empty() && prefixSum[i] - prefixSum[dq.front()] >= k) {
                minLength = min(minLength, i - dq.front());
                dq.pop_front();
            }
            
            // Maintain monotonic increasing deque
            while (!dq.empty() && prefixSum[i] <= prefixSum[dq.back()]) {
                dq.pop_back();
            }
            
            dq.push_back(i);
        }
        
        return minLength == INT_MAX ? -1 : minLength;
    }
};

int main() {
    // Test case 1
    vector<int> nums1 = {1};
    cout << "Test 1: " << Task178::shortestSubarray(nums1, 1) << endl; // Expected: 1
    
    // Test case 2
    vector<int> nums2 = {1, 2};
    cout << "Test 2: " << Task178::shortestSubarray(nums2, 4) << endl; // Expected: -1
    
    // Test case 3
    vector<int> nums3 = {2, -1, 2};
    cout << "Test 3: " << Task178::shortestSubarray(nums3, 3) << endl; // Expected: 3
    
    // Test case 4
    vector<int> nums4 = {84, -37, 32, 40, 95};
    cout << "Test 4: " << Task178::shortestSubarray(nums4, 167) << endl; // Expected: 3
    
    // Test case 5
    vector<int> nums5 = {-28, 81, -20, 28, -29};
    cout << "Test 5: " << Task178::shortestSubarray(nums5, 89) << endl; // Expected: 3
    
    return 0;
}
