#include <iostream>
#include <vector>
#include <functional>

// Definition for a binary tree node.
struct TreeNode {
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
};

// Helper for preorder traversal
void preorderHelper(TreeNode* node, std::vector<int>& result) {
    if (node == nullptr) {
        return;
    }
    result.push_back(node->val);
    preorderHelper(node->left, result);
    preorderHelper(node->right, result);
}

// Preorder Traversal: Root -> Left -> Right
std::vector<int> preorderTraversal(TreeNode* root) {
    std::vector<int> result;
    preorderHelper(root, result);
    return result;
}

// Helper for inorder traversal
void inorderHelper(TreeNode* node, std::vector<int>& result) {
    if (node == nullptr) {
        return;
    }
    inorderHelper(node->left, result);
    result.push_back(node->val);
    inorderHelper(node->right, result);
}

// Inorder Traversal: Left -> Root -> Right
std::vector<int> inorderTraversal(TreeNode* root) {
    std::vector<int> result;
    inorderHelper(root, result);
    return result;
}

// Helper for postorder traversal
void postorderHelper(TreeNode* node, std::vector<int>& result) {
    if (node == nullptr) {
        return;
    }
    postorderHelper(node->left, result);
    postorderHelper(node->right, result);
    result.push_back(node->val);
}

// Postorder Traversal: Left -> Right -> Root
std::vector<int> postorderTraversal(TreeNode* root) {
    std::vector<int> result;
    postorderHelper(root, result);
    return result;
}

// Helper function to print a vector
void printVector(const std::vector<int>& vec) {
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i];
        if (i < vec.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]";
}

// Helper function to free the tree memory
void freeTree(TreeNode* node) {
    if (node == nullptr) {
        return;
    }
    freeTree(node->left);
    freeTree(node->right);
    delete node;
}


int main() {
    // --- Test Case 1: Standard Tree ---
    std::cout << "--- Test Case 1 ---" << std::endl;
    TreeNode* root1 = new TreeNode(1);
    root1->left = new TreeNode(2);
    root1->right = new TreeNode(3);
    root1->left->left = new TreeNode(4);
    root1->left->right = new TreeNode(5);
    std::cout << "Preorder:  "; printVector(preorderTraversal(root1)); std::cout << std::endl;
    std::cout << "Inorder:   "; printVector(inorderTraversal(root1)); std::cout << std::endl;
    std::cout << "Postorder: "; printVector(postorderTraversal(root1)); std::cout << std::endl;
    freeTree(root1);
    std::cout << std::endl;

    // --- Test Case 2: Null Tree ---
    std::cout << "--- Test Case 2 ---" << std::endl;
    TreeNode* root2 = nullptr;
    std::cout << "Preorder:  "; printVector(preorderTraversal(root2)); std::cout << std::endl;
    std::cout << "Inorder:   "; printVector(inorderTraversal(root2)); std::cout << std::endl;
    std::cout << "Postorder: "; printVector(postorderTraversal(root2)); std::cout << std::endl;
    std::cout << std::endl;

    // --- Test Case 3: Single Node Tree ---
    std::cout << "--- Test Case 3 ---" << std::endl;
    TreeNode* root3 = new TreeNode(10);
    std::cout << "Preorder:  "; printVector(preorderTraversal(root3)); std::cout << std::endl;
    std::cout << "Inorder:   "; printVector(inorderTraversal(root3)); std::cout << std::endl;
    std::cout << "Postorder: "; printVector(postorderTraversal(root3)); std::cout << std::endl;
    freeTree(root3);
    std::cout << std::endl;

    // --- Test Case 4: Left-skewed Tree ---
    std::cout << "--- Test Case 4 ---" << std::endl;
    TreeNode* root4 = new TreeNode(4);
    root4->left = new TreeNode(3);
    root4->left->left = new TreeNode(2);
    root4->left->left->left = new TreeNode(1);
    std::cout << "Preorder:  "; printVector(preorderTraversal(root4)); std::cout << std::endl;
    std::cout << "Inorder:   "; printVector(inorderTraversal(root4)); std::cout << std::endl;
    std::cout << "Postorder: "; printVector(postorderTraversal(root4)); std::cout << std::endl;
    freeTree(root4);
    std::cout << std::endl;

    // --- Test Case 5: Complex Tree ---
    std::cout << "--- Test Case 5 ---" << std::endl;
    TreeNode* root5 = new TreeNode(10);
    root5->left = new TreeNode(5);
    root5->right = new TreeNode(15);
    root5->left->left = new TreeNode(3);
    root5->left->right = new TreeNode(7);
    root5->right->right = new TreeNode(18);
    std::cout << "Preorder:  "; printVector(preorderTraversal(root5)); std::cout << std::endl;
    std::cout << "Inorder:   "; printVector(inorderTraversal(root5)); std::cout << std::endl;
    std::cout << "Postorder: "; printVector(postorderTraversal(root5)); std::cout << std::endl;
    freeTree(root5);
    std::cout << std::endl;

    return 0;
}