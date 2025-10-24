
#include <iostream>
#include <vector>
#include <queue>
using namespace std;

class Task147 {
public:
    static int findKthLargest(vector<int>& nums, int k) {
        priority_queue<int, vector<int>, greater<int>> minHeap;
        
        for (int num : nums) {
            minHeap.push(num);
            if (minHeap.size() > k) {
                minHeap.pop();
            }
        }
        
        return minHeap.top();
    }
};

int main() {
    // Test case 1
    vector<int> nums1 = {3, 2, 1, 5, 6, 4};
    int k1 = 2;
    cout << "Test 1: " << Task147::findKthLargest(nums1, k1) << endl; // Expected: 5
    
    // Test case 2
    vector<int> nums2 = {3, 2, 3, 1, 2, 4, 5, 5, 6};
    int k2 = 4;
    cout << "Test 2: " << Task147::findKthLargest(nums2, k2) << endl; // Expected: 4
    
    // Test case 3
    vector<int> nums3 = {7, 10, 4, 3, 20, 15};
    int k3 = 3;
    cout << "Test 3: " << Task147::findKthLargest(nums3, k3) << endl; // Expected: 10
    
    // Test case 4
    vector<int> nums4 = {1};
    int k4 = 1;
    cout << "Test 4: " << Task147::findKthLargest(nums4, k4) << endl; // Expected: 1
    
    // Test case 5
    vector<int> nums5 = {-1, -5, -3, -7, -2};
    int k5 = 2;
    cout << "Test 5: " << Task147::findKthLargest(nums5, k5) << endl; // Expected: -2
    
    return 0;
}
