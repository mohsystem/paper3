
#include <iostream>
#include <vector>
using namespace std;

struct TreeNode {
    int val;
    TreeNode* left;
    TreeNode* right;
    
    TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
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
    TreeNode* tree1 = new TreeNode(1);
    tree1->right = new TreeNode(2);
    tree1->right->left = new TreeNode(3);
    cout << "Test Case 1:" << endl;
    cout << "Preorder: "; printVector(preorderTraversal(tree1));
    cout << "Inorder: "; printVector(inorderTraversal(tree1));
    cout << "Postorder: "; printVector(postorderTraversal(tree1));
    
    // Test Case 2: Balanced tree
    TreeNode* tree2 = new TreeNode(1);
    tree2->left = new TreeNode(2);
    tree2->right = new TreeNode(3);
    tree2->left->left = new TreeNode(4);
    tree2->left->right = new TreeNode(5);
    cout << "\\nTest Case 2:" << endl;
    cout << "Preorder: "; printVector(preorderTraversal(tree2));
    cout << "Inorder: "; printVector(inorderTraversal(tree2));
    cout << "Postorder: "; printVector(postorderTraversal(tree2));
    
    // Test Case 3: Single node
    TreeNode* tree3 = new TreeNode(42);
    cout << "\\nTest Case 3:" << endl;
    cout << "Preorder: "; printVector(preorderTraversal(tree3));
    cout << "Inorder: "; printVector(inorderTraversal(tree3));
    cout << "Postorder: "; printVector(postorderTraversal(tree3));
    
    // Test Case 4: Empty tree
    TreeNode* tree4 = nullptr;
    cout << "\\nTest Case 4:" << endl;
    cout << "Preorder: "; printVector(preorderTraversal(tree4));
    cout << "Inorder: "; printVector(inorderTraversal(tree4));
    cout << "Postorder: "; printVector(postorderTraversal(tree4));
    
    // Test Case 5: Left skewed tree
    TreeNode* tree5 = new TreeNode(5);
    tree5->left = new TreeNode(4);
    tree5->left->left = new TreeNode(3);
    tree5->left->left->left = new TreeNode(2);
    tree5->left->left->left->left = new TreeNode(1);
    cout << "\\nTest Case 5:" << endl;
    cout << "Preorder: "; printVector(preorderTraversal(tree5));
    cout << "Inorder: "; printVector(inorderTraversal(tree5));
    cout << "Postorder: "; printVector(postorderTraversal(tree5));
    
    return 0;
}
