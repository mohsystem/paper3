
#include <iostream>
#include <vector>
#include <algorithm>
#include <climits>

using namespace std;

class Task185 {
public:
    static double findMedianSortedArrays(vector<int>& nums1, vector<int>& nums2) {
        if (nums1.size() > nums2.size()) {
            return findMedianSortedArrays(nums2, nums1);
        }
        
        int m = nums1.size();
        int n = nums2.size();
        int low = 0, high = m;
        
        while (low <= high) {
            int partitionX = (low + high) / 2;
            int partitionY = (m + n + 1) / 2 - partitionX;
            
            int maxLeftX = (partitionX == 0) ? INT_MIN : nums1[partitionX - 1];
            int minRightX = (partitionX == m) ? INT_MAX : nums1[partitionX];
            
            int maxLeftY = (partitionY == 0) ? INT_MIN : nums2[partitionY - 1];
            int minRightY = (partitionY == n) ? INT_MAX : nums2[partitionY];
            
            if (maxLeftX <= minRightY && maxLeftY <= minRightX) {
                if ((m + n) % 2 == 0) {
                    return (max(maxLeftX, maxLeftY) + min(minRightX, minRightY)) / 2.0;
                } else {
                    return max(maxLeftX, maxLeftY);
                }
            } else if (maxLeftX > minRightY) {
                high = partitionX - 1;
            } else {
                low = partitionX + 1;
            }
        }
        
        throw invalid_argument("Input arrays are not sorted");
    }
};

int main() {
    // Test case 1
    vector<int> nums1_1 = {1, 3};
    vector<int> nums2_1 = {2};
    cout << "Test 1: " << Task185::findMedianSortedArrays(nums1_1, nums2_1) << endl;
    
    // Test case 2
    vector<int> nums1_2 = {1, 2};
    vector<int> nums2_2 = {3, 4};
    cout << "Test 2: " << Task185::findMedianSortedArrays(nums1_2, nums2_2) << endl;
    
    // Test case 3
    vector<int> nums1_3 = {};
    vector<int> nums2_3 = {1};
    cout << "Test 3: " << Task185::findMedianSortedArrays(nums1_3, nums2_3) << endl;
    
    // Test case 4
    vector<int> nums1_4 = {1, 2, 3, 4, 5};
    vector<int> nums2_4 = {6, 7, 8, 9, 10};
    cout << "Test 4: " << Task185::findMedianSortedArrays(nums1_4, nums2_4) << endl;
    
    // Test case 5
    vector<int> nums1_5 = {1, 3, 5, 7};
    vector<int> nums2_5 = {2, 4, 6};
    cout << "Test 5: " << Task185::findMedianSortedArrays(nums1_5, nums2_5) << endl;
    
    return 0;
}
