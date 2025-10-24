
#include <iostream>
#include <vector>
#include <limits>

using namespace std;

struct TreeNode {
    int val;
    TreeNode* left;
    TreeNode* right;
    
    TreeNode(int v) : val(v), left(nullptr), right(nullptr) {
        if (v < INT_MIN || v > INT_MAX) {
            throw invalid_argument("Invalid node value");
        }
    }
    
    ~TreeNode() {
        delete left;
        delete right;
    }
};

void preorderHelper(TreeNode* node, vector<int>& result) {
    if (node == nullptr) {
        return;
    }
    result.push_back(node->val);
    preorderHelper(node->left, result);
    preorderHelper(node->right, result);
}

vector<int> preorderTraversal(TreeNode* root) {
    vector<int> result;
    if (root == nullptr) {
        return result;
    }
    preorderHelper(root, result);
    return result;
}

void inorderHelper(TreeNode* node, vector<int>& result) {
    if (node == nullptr) {
        return;
    }
    inorderHelper(node->left, result);
    result.push_back(node->val);
    inorderHelper(node->right, result);
}

vector<int> inorderTraversal(TreeNode* root) {
    vector<int> result;
    if (root == nullptr) {
        return result;
    }
    inorderHelper(root, result);
    return result;
}

void postorderHelper(TreeNode* node, vector<int>& result) {
    if (node == nullptr) {
        return;
    }
    postorderHelper(node->left, result);
    postorderHelper(node->right, result);
    result.push_back(node->val);
}

vector<int> postorderTraversal(TreeNode* root) {
    vector<int> result;
    if (root == nullptr) {
        return result;
    }
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
    // Test Case 1: Single node tree
    TreeNode* test1 = new TreeNode(1);
    cout << "Test 1 - Preorder: ";
    printVector(preorderTraversal(test1));
    cout << "Test 1 - Inorder: ";
    printVector(inorderTraversal(test1));
    cout << "Test 1 - Postorder: ";
    printVector(postorderTraversal(test1));
    delete test1;
    
    // Test Case 2: Balanced tree
    TreeNode* test2 = new TreeNode(1);
    test2->left = new TreeNode(2);
    test2->right = new TreeNode(3);
    test2->left->left = new TreeNode(4);
    test2->left->right = new TreeNode(5);
    cout << "\\nTest 2 - Preorder: ";
    printVector(preorderTraversal(test2));
    cout << "Test 2 - Inorder: ";
    printVector(inorderTraversal(test2));
    cout << "Test 2 - Postorder: ";
    printVector(postorderTraversal(test2));
    delete test2;
    
    // Test Case 3: Left-skewed tree
    TreeNode* test3 = new TreeNode(1);
    test3->left = new TreeNode(2);
    test3->left->left = new TreeNode(3);
    cout << "\\nTest 3 - Preorder: ";
    printVector(preorderTraversal(test3));
    cout << "Test 3 - Inorder: ";
    printVector(inorderTraversal(test3));
    cout << "Test 3 - Postorder: ";
    printVector(postorderTraversal(test3));
    delete test3;
    
    // Test Case 4: Right-skewed tree
    TreeNode* test4 = new TreeNode(1);
    test4->right = new TreeNode(2);
    test4->right->right = new TreeNode(3);
    cout << "\\nTest 4 - Preorder: ";
    printVector(preorderTraversal(test4));
    cout << "Test 4 - Inorder: ";
    printVector(inorderTraversal(test4));
    cout << "Test 4 - Postorder: ";
    printVector(postorderTraversal(test4));
    delete test4;
    
    // Test Case 5: Empty tree
    TreeNode* test5 = nullptr;
    cout << "\\nTest 5 - Preorder: ";
    printVector(preorderTraversal(test5));
    cout << "Test 5 - Inorder: ";
    printVector(inorderTraversal(test5));
    cout << "Test 5 - Postorder: ";
    printVector(postorderTraversal(test5));
    
    return 0;
}
