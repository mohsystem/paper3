
#include <iostream>
#include <vector>
using namespace std;

struct TreeNode {
    int val;
    int count;
    int leftSize;
    TreeNode* left;
    TreeNode* right;
    
    TreeNode(int v) : val(v), count(1), leftSize(0), left(nullptr), right(nullptr) {}
};

class Task169 {
public:
    static TreeNode* insert(TreeNode* node, int val, vector<int>& result, int index, int preSum) {
        if (node == nullptr) {
            node = new TreeNode(val);
            result[index] = preSum;
        } else if (node->val == val) {
            node->count++;
            result[index] = preSum + node->leftSize;
        } else if (node->val > val) {
            node->leftSize++;
            node->left = insert(node->left, val, result, index, preSum);
        } else {
            node->right = insert(node->right, val, result, index, preSum + node->leftSize + node->count);
        }
        return node;
    }
    
    static vector<int> countSmaller(vector<int>& nums) {
        if (nums.empty()) {
            return vector<int>();
        }
        
        vector<int> result(nums.size());
        TreeNode* root = nullptr;
        
        for (int i = nums.size() - 1; i >= 0; i--) {
            root = insert(root, nums[i], result, i, 0);
        }
        
        return result;
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
