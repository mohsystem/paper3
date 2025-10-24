
#include <iostream>
#include <vector>
#include <deque>
using namespace std;

class Task179 {
public:
    static vector<int> maxSlidingWindow(vector<int>& nums, int k) {
        if (nums.empty() || k <= 0) {
            return vector<int>();
        }
        
        int n = nums.size();
        vector<int> result;
        deque<int> dq;
        
        for (int i = 0; i < n; i++) {
            // Remove elements outside the current window
            while (!dq.empty() && dq.front() < i - k + 1) {
                dq.pop_front();
            }
            
            // Remove elements smaller than current element from the back
            while (!dq.empty() && nums[dq.back()] < nums[i]) {
                dq.pop_back();
            }
            
            dq.push_back(i);
            
            // Add to result when window is complete
            if (i >= k - 1) {
                result.push_back(nums[dq.front()]);
            }
        }
        
        return result;
    }
};

int main() {
    // Test case 1
    vector<int> nums1 = {1, 3, -1, -3, 5, 3, 6, 7};
    int k1 = 3;
    vector<int> result1 = Task179::maxSlidingWindow(nums1, k1);
    cout << "Test 1: ";
    for (int val : result1) cout << val << " ";
    cout << endl;
    
    // Test case 2
    vector<int> nums2 = {1};
    int k2 = 1;
    vector<int> result2 = Task179::maxSlidingWindow(nums2, k2);
    cout << "Test 2: ";
    for (int val : result2) cout << val << " ";
    cout << endl;
    
    // Test case 3
    vector<int> nums3 = {1, -1};
    int k3 = 1;
    vector<int> result3 = Task179::maxSlidingWindow(nums3, k3);
    cout << "Test 3: ";
    for (int val : result3) cout << val << " ";
    cout << endl;
    
    // Test case 4
    vector<int> nums4 = {9, 11};
    int k4 = 2;
    vector<int> result4 = Task179::maxSlidingWindow(nums4, k4);
    cout << "Test 4: ";
    for (int val : result4) cout << val << " ";
    cout << endl;
    
    // Test case 5
    vector<int> nums5 = {4, -2, 5, 1, 3};
    int k5 = 2;
    vector<int> result5 = Task179::maxSlidingWindow(nums5, k5);
    cout << "Test 5: ";
    for (int val : result5) cout << val << " ";
    cout << endl;
    
    return 0;
}
