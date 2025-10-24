
#include <vector>
#include <iostream>
using namespace std;

class Task169 {
public:
    static vector<int> countSmaller(vector<int>& nums) {
        if (nums.empty()) {
            return {};
        }
        
        int n = nums.size();
        vector<int> result(n, 0);
        vector<int> indices(n);
        
        for (int i = 0; i < n; i++) {
            indices[i] = i;
        }
        
        mergeSort(nums, indices, result, 0, n - 1);
        return result;
    }
    
private:
    static void mergeSort(vector<int>& nums, vector<int>& indices, vector<int>& result, int left, int right) {
        if (left >= right) {
            return;
        }
        
        int mid = left + (right - left) / 2;
        mergeSort(nums, indices, result, left, mid);
        mergeSort(nums, indices, result, mid + 1, right);
        merge(nums, indices, result, left, mid, right);
    }
    
    static void merge(vector<int>& nums, vector<int>& indices, vector<int>& result, int left, int mid, int right) {
        vector<int> temp;
        int i = left, j = mid + 1;
        int rightCount = 0;
        
        while (i <= mid && j <= right) {
            if (nums[indices[j]] < nums[indices[i]]) {
                temp.push_back(indices[j++]);
                rightCount++;
            } else {
                result[indices[i]] += rightCount;
                temp.push_back(indices[i++]);
            }
        }
        
        while (i <= mid) {
            result[indices[i]] += rightCount;
            temp.push_back(indices[i++]);
        }
        
        while (j <= right) {
            temp.push_back(indices[j++]);
        }
        
        for (int k = 0; k < temp.size(); k++) {
            indices[left + k] = temp[k];
        }
    }
};

int main() {
    // Test case 1
    vector<int> nums1 = {5, 2, 6, 1};
    vector<int> result1 = Task169::countSmaller(nums1);
    cout << "Test 1: ";
    for (int val : result1) cout << val << " ";
    cout << endl;
    
    // Test case 2
    vector<int> nums2 = {-1};
    vector<int> result2 = Task169::countSmaller(nums2);
    cout << "Test 2: ";
    for (int val : result2) cout << val << " ";
    cout << endl;
    
    // Test case 3
    vector<int> nums3 = {-1, -1};
    vector<int> result3 = Task169::countSmaller(nums3);
    cout << "Test 3: ";
    for (int val : result3) cout << val << " ";
    cout << endl;
    
    // Test case 4
    vector<int> nums4 = {1, 2, 3, 4, 5};
    vector<int> result4 = Task169::countSmaller(nums4);
    cout << "Test 4: ";
    for (int val : result4) cout << val << " ";
    cout << endl;
    
    // Test case 5
    vector<int> nums5 = {5, 4, 3, 2, 1};
    vector<int> result5 = Task169::countSmaller(nums5);
    cout << "Test 5: ";
    for (int val : result5) cout << val << " ";
    cout << endl;
    
    return 0;
}
