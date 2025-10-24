
#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

class Task171 {
public:
    static int reversePairs(vector<int>& nums) {
        if (nums.empty() || nums.size() < 2) {
            return 0;
        }
        return mergeSort(nums, 0, nums.size() - 1);
    }
    
private:
    static int mergeSort(vector<int>& nums, int left, int right) {
        if (left >= right) {
            return 0;
        }
        
        int mid = left + (right - left) / 2;
        int count = mergeSort(nums, left, mid) + mergeSort(nums, mid + 1, right);
        
        // Count reverse pairs
        int j = mid + 1;
        for (int i = left; i <= mid; i++) {
            while (j <= right && (long long)nums[i] > 2LL * (long long)nums[j]) {
                j++;
            }
            count += j - (mid + 1);
        }
        
        // Merge
        vector<int> temp;
        int i = left;
        j = mid + 1;
        
        while (i <= mid && j <= right) {
            if (nums[i] <= nums[j]) {
                temp.push_back(nums[i++]);
            } else {
                temp.push_back(nums[j++]);
            }
        }
        
        while (i <= mid) {
            temp.push_back(nums[i++]);
        }
        
        while (j <= right) {
            temp.push_back(nums[j++]);
        }
        
        copy(temp.begin(), temp.end(), nums.begin() + left);
        
        return count;
    }
};

int main() {
    // Test case 1
    vector<int> test1 = {1, 3, 2, 3, 1};
    cout << "Test 1: " << Task171::reversePairs(test1) << endl; // Expected: 2
    
    // Test case 2
    vector<int> test2 = {2, 4, 3, 5, 1};
    cout << "Test 2: " << Task171::reversePairs(test2) << endl; // Expected: 3
    
    // Test case 3
    vector<int> test3 = {5, 4, 3, 2, 1};
    cout << "Test 3: " << Task171::reversePairs(test3) << endl; // Expected: 4
    
    // Test case 4
    vector<int> test4 = {1, 2, 3, 4, 5};
    cout << "Test 4: " << Task171::reversePairs(test4) << endl; // Expected: 0
    
    // Test case 5
    vector<int> test5 = {2147483647, 2147483647, 2147483647, 2147483647, 2147483647, 2147483647};
    cout << "Test 5: " << Task171::reversePairs(test5) << endl; // Expected: 0
    
    return 0;
}
