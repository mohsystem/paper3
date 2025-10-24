
#include <iostream>
#include <vector>
using namespace std;

struct Node {
    int val;
    int count;
    Node* left;
    Node* right;
    
    Node(int v) : val(v), count(0), left(nullptr), right(nullptr) {}
};

class Task169 {
public:
    static Node* insert(Node* node, int val, vector<int>& result, int index, int preSum) {
        if (node == nullptr) {
            node = new Node(val);
            result[index] = preSum;
        } else if (node->val > val) {
            node->count++;
            node->left = insert(node->left, val, result, index, preSum);
        } else {
            node->right = insert(node->right, val, result, index, preSum + node->count + (node->val < val ? 1 : 0));
        }
        return node;
    }
    
    static vector<int> countSmaller(vector<int>& nums) {
        if (nums.empty()) return {};
        
        vector<int> result(nums.size(), 0);
        Node* root = nullptr;
        
        for (int i = nums.size() - 1; i >= 0; i--) {
            root = insert(root, nums[i], result, i, 0);
        }
        
        return result;
    }
};

int main() {
    // Test case 1
    vector<int> nums1 = {5, 2, 6, 1};
    vector<int> res1 = Task169::countSmaller(nums1);
    cout << "Test 1: ";
    for (int n : res1) cout << n << " ";
    cout << endl;
    
    // Test case 2
    vector<int> nums2 = {-1};
    vector<int> res2 = Task169::countSmaller(nums2);
    cout << "Test 2: ";
    for (int n : res2) cout << n << " ";
    cout << endl;
    
    // Test case 3
    vector<int> nums3 = {-1, -1};
    vector<int> res3 = Task169::countSmaller(nums3);
    cout << "Test 3: ";
    for (int n : res3) cout << n << " ";
    cout << endl;
    
    // Test case 4
    vector<int> nums4 = {1, 2, 3, 4, 5};
    vector<int> res4 = Task169::countSmaller(nums4);
    cout << "Test 4: ";
    for (int n : res4) cout << n << " ";
    cout << endl;
    
    // Test case 5
    vector<int> nums5 = {5, 4, 3, 2, 1};
    vector<int> res5 = Task169::countSmaller(nums5);
    cout << "Test 5: ";
    for (int n : res5) cout << n << " ";
    cout << endl;
    
    return 0;
}
