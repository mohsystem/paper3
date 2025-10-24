
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

class Task170 {
private:
    static int countWhileMergeSort(vector<long long>& sums, int start, int end, int lower, int upper) {
        if (end - start <= 1) return 0;
        
        int mid = (start + end) / 2;
        int count = countWhileMergeSort(sums, start, mid, lower, upper) 
                  + countWhileMergeSort(sums, mid, end, lower, upper);
        
        int j = mid, k = mid, t = mid;
        vector<long long> cache;
        
        for (int i = start; i < mid; i++) {
            while (k < end && sums[k] - sums[i] < lower) k++;
            while (j < end && sums[j] - sums[i] <= upper) j++;
            count += j - k;
            
            while (t < end && sums[t] < sums[i]) cache.push_back(sums[t++]);
            cache.push_back(sums[i]);
        }
        
        copy(cache.begin(), cache.end(), sums.begin() + start);
        
        return count;
    }

public:
    static int countRangeSum(vector<int>& nums, int lower, int upper) {
        int n = nums.size();
        vector<long long> prefixSum(n + 1, 0);
        
        for (int i = 0; i < n; i++) {
            prefixSum[i + 1] = prefixSum[i] + nums[i];
        }
        
        return countWhileMergeSort(prefixSum, 0, n + 1, lower, upper);
    }
};

int main() {
    // Test case 1
    vector<int> nums1 = {-2, 5, -1};
    cout << "Test 1: " << Task170::countRangeSum(nums1, -2, 2) << endl; // Expected: 3
    
    // Test case 2
    vector<int> nums2 = {0};
    cout << "Test 2: " << Task170::countRangeSum(nums2, 0, 0) << endl; // Expected: 1
    
    // Test case 3
    vector<int> nums3 = {-2, 5, -1, 3, -2};
    cout << "Test 3: " << Task170::countRangeSum(nums3, -2, 3) << endl; // Expected: 8
    
    // Test case 4
    vector<int> nums4 = {1, 2, 3, 4, 5};
    cout << "Test 4: " << Task170::countRangeSum(nums4, 3, 7) << endl; // Expected: 6
    
    // Test case 5
    vector<int> nums5 = {-1, -2, -3};
    cout << "Test 5: " << Task170::countRangeSum(nums5, -5, -1) << endl; // Expected: 5
    
    return 0;
}
