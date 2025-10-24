
#include <iostream>
#include <vector>
using namespace std;

struct TreeNode {
    int val;
    TreeNode* left;
    TreeNode* right;
    
    TreeNode(int v) : val(v), left(nullptr), right(nullptr) {}
};

void preorderHelper(TreeNode* node, vector<int>& result) {
    if (node == nullptr) return;
    result.push_back(node->val);
    preorderHelper(node->left, result);
    preorderHelper(node->right, result);
}

vector<int> preorderTraversal(TreeNode* root) {
    vector<int> result;
    preorderHelper(root, result);
    return result;
}

void inorderHelper(TreeNode* node, vector<int>& result) {
    if (node == nullptr) return;
    inorderHelper(node->left, result);
    result.push_back(node->val);
    inorderHelper(node->right, result);
}

vector<int> inorderTraversal(TreeNode* root) {
    vector<int> result;
    inorderHelper(root, result);
    return result;
}

void postorderHelper(TreeNode* node, vector<int>& result) {
    if (node == nullptr) return;
    postorderHelper(node->left, result);
    postorderHelper(node->right, result);
    result.push_back(node->val);
}

vector<int> postorderTraversal(TreeNode* root) {
    vector<int> result;
    postorderHelper(root, result);
    return result;
}

void printVector(const vector<int>& vec) {
    cout << "[";
    for (size_t i = 0; i < vec.size(); i++) {
        cout << vec[i];
        if (i < vec.size() - 1) cout << ", ";
    }
    cout << "]" << endl;
}

int main() {
    // Test Case 1: Simple tree
    TreeNode* root1 = new TreeNode(1);
    root1->left = new TreeNode(2);
    root1->right = new TreeNode(3);
    cout << "Test Case 1:" << endl;
    cout << "Preorder: "; printVector(preorderTraversal(root1));
    cout << "Inorder: "; printVector(inorderTraversal(root1));
    cout << "Postorder: "; printVector(postorderTraversal(root1));
    cout << endl;
    
    // Test Case 2: Left-skewed tree
    TreeNode* root2 = new TreeNode(1);
    root2->left = new TreeNode(2);
    root2->left->left = new TreeNode(3);
    cout << "Test Case 2:" << endl;
    cout << "Preorder: "; printVector(preorderTraversal(root2));
    cout << "Inorder: "; printVector(inorderTraversal(root2));
    cout << "Postorder: "; printVector(postorderTraversal(root2));
    cout << endl;
    
    // Test Case 3: Right-skewed tree
    TreeNode* root3 = new TreeNode(1);
    root3->right = new TreeNode(2);
    root3->right->right = new TreeNode(3);
    cout << "Test Case 3:" << endl;
    cout << "Preorder: "; printVector(preorderTraversal(root3));
    cout << "Inorder: "; printVector(inorderTraversal(root3));
    cout << "Postorder: "; printVector(postorderTraversal(root3));
    cout << endl;
    
    // Test Case 4: Complete binary tree
    TreeNode* root4 = new TreeNode(1);
    root4->left = new TreeNode(2);
    root4->right = new TreeNode(3);
    root4->left->left = new TreeNode(4);
    root4->left->right = new TreeNode(5);
    root4->right->left = new TreeNode(6);
    root4->right->right = new TreeNode(7);
    cout << "Test Case 4:" << endl;
    cout << "Preorder: "; printVector(preorderTraversal(root4));
    cout << "Inorder: "; printVector(inorderTraversal(root4));
    cout << "Postorder: "; printVector(postorderTraversal(root4));
    cout << endl;
    
    // Test Case 5: Empty tree
    TreeNode* root5 = nullptr;
    cout << "Test Case 5:" << endl;
    cout << "Preorder: "; printVector(preorderTraversal(root5));
    cout << "Inorder: "; printVector(inorderTraversal(root5));
    cout << "Postorder: "; printVector(postorderTraversal(root5));
    
    return 0;
}
