
#include <iostream>
#include <vector>
#include <algorithm>
#include <climits>

using namespace std;

class Solution {
private:
    /* Merge helper that counts cross-boundary reverse pairs
     * Security: Uses vector bounds checking and prevents integer overflow
     */
    int mergeAndCount(vector<int>& nums, vector<int>& temp, int left, int mid, int right) {
        /* Input validation */
        if (left < 0 || mid < left || right < mid || right >= static_cast<int>(nums.size())) {
            return 0;
        }
        
        int count = 0;
        
        /* Count reverse pairs where i is in left half and j is in right half
         * Security: Use long long to prevent integer overflow
         */
        int j = mid + 1;
        for (int i = left; i <= mid; i++) {
            /* Prevent overflow by casting to long long before multiplication */
            while (j <= right && static_cast<long long>(nums[i]) > 2LL * static_cast<long long>(nums[j])) {
                j++;
            }
            count += (j - (mid + 1));
        }
        
        /* Merge the two sorted halves */
        int i = left;
        j = mid + 1;
        int k = left;
        
        while (i <= mid && j <= right) {
            if (nums[i] <= nums[j]) {
                temp[k++] = nums[i++];
            } else {
                temp[k++] = nums[j++];
            }
        }
        
        /* Copy remaining elements */
        while (i <= mid) {
            temp[k++] = nums[i++];
        }
        
        while (j <= right) {
            temp[k++] = nums[j++];
        }
        
        /* Copy back to original array with bounds checking */
        for (int idx = left; idx <= right; idx++) {
            nums[idx] = temp[idx];
        }
        
        return count;
    }
    
    /* Recursive merge sort that counts reverse pairs
     * Security: Validates inputs and prevents overflow
     */
    int mergeSortAndCount(vector<int>& nums, vector<int>& temp, int left, int right) {
        /* Boundary check */
        if (left >= right) {
            return 0;
        }
        
        /* Prevent integer overflow in mid calculation */
        int mid = left + (right - left) / 2;
        
        int count = 0;
        count += mergeSortAndCount(nums, temp, left, mid);
        count += mergeSortAndCount(nums, temp, mid + 1, right);
        count += mergeAndCount(nums, temp, left, mid, right);
        
        return count;
    }
    
public:
    int reversePairs(vector<int>& nums) {
        /* Input validation */
        if (nums.empty() || nums.size() > 50000) {
            return 0;
        }
        
        /* Create temporary vector for merging */
        vector<int> temp(nums.size(), 0);
        
        /* Count reverse pairs using merge sort */
        return mergeSortAndCount(nums, temp, 0, static_cast<int>(nums.size()) - 1);
    }
};

/* Test driver with 5 test cases */
int main() {
    Solution solution;
    
    /* Test case 1: Example 1 from problem */
    vector<int> test1 = {1, 3, 2, 3, 1};
    cout << "Test 1: [1,3,2,3,1] -> " << solution.reversePairs(test1) << endl;
    
    /* Test case 2: Example 2 from problem */
    vector<int> test2 = {2, 4, 3, 5, 1};
    cout << "Test 2: [2,4,3,5,1] -> " << solution.reversePairs(test2) << endl;
    
    /* Test case 3: Single element */
    vector<int> test3 = {1};
    cout << "Test 3: [1] -> " << solution.reversePairs(test3) << endl;
    
    /* Test case 4: Already sorted */
    vector<int> test4 = {1, 2, 3, 4, 5};
    cout << "Test 4: [1,2,3,4,5] -> " << solution.reversePairs(test4) << endl;
    
    /* Test case 5: With negative numbers and edge values */
    vector<int> test5 = {2147483647, -2147483648, 0, 1};
    cout << "Test 5: [2147483647,-2147483648,0,1] -> " << solution.reversePairs(test5) << endl;
    
    return 0;
}
