#include <iostream>
#include <vector>
#include <memory>
#include <string>

struct TreeNode {
    int val;
    std::unique_ptr<TreeNode> left;
    std::unique_ptr<TreeNode> right;

    TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
};

// Preorder traversal: Root -> Left -> Right
void preorderTraversal(const TreeNode* root, std::vector<int>& result) {
    if (root == nullptr) {
        return;
    }
    result.push_back(root->val);
    preorderTraversal(root->left.get(), result);
    preorderTraversal(root->right.get(), result);
}

// Inorder traversal: Left -> Root -> Right
void inorderTraversal(const TreeNode* root, std::vector<int>& result) {
    if (root == nullptr) {
        return;
    }
    inorderTraversal(root->left.get(), result);
    result.push_back(root->val);
    inorderTraversal(root->right.get(), result);
}

// Postorder traversal: Left -> Right -> Root
void postorderTraversal(const TreeNode* root, std::vector<int>& result) {
    if (root == nullptr) {
        return;
    }
    postorderTraversal(root->left.get(), result);
    postorderTraversal(root->right.get(), result);
    result.push_back(root->val);
}

void printVector(const std::string& prefix, const std::vector<int>& vec) {
    std::cout << prefix;
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << (i == vec.size() - 1 ? "" : ", ");
    }
    std::cout << "]" << std::endl;
}

void runTestCase(const std::string& name, const std::unique_ptr<TreeNode>& root) {
    std::cout << "--- " << name << " ---" << std::endl;
    
    std::vector<int> preorderResult;
    preorderTraversal(root.get(), preorderResult);
    printVector("Preorder:  [", preorderResult);

    std::vector<int> inorderResult;
    inorderTraversal(root.get(), inorderResult);
    printVector("Inorder:   [", inorderResult);

    std::vector<int> postorderResult;
    postorderTraversal(root.get(), postorderResult);
    printVector("Postorder: [", postorderResult);
    
    std::cout << std::endl;
}

int main() {
    // Test Case 1: A balanced binary tree
    auto root1 = std::make_unique<TreeNode>(4);
    root1->left = std::make_unique<TreeNode>(2);
    root1->right = std::make_unique<TreeNode>(7);
    root1->left->left = std::make_unique<TreeNode>(1);
    root1->left->right = std::make_unique<TreeNode>(3);
    root1->right->left = std::make_unique<TreeNode>(6);
    root1->right->right = std::make_unique<TreeNode>(9);
    runTestCase("Test Case 1: Balanced Tree", root1);

    // Test Case 2: A left-skewed tree
    auto root2 = std::make_unique<TreeNode>(3);
    root2->left = std::make_unique<TreeNode>(2);
    root2->left->left = std::make_unique<TreeNode>(1);
    runTestCase("Test Case 2: Left-Skewed Tree", root2);

    // Test Case 3: A right-skewed tree
    auto root3 = std::make_unique<TreeNode>(1);
    root3->right = std::make_unique<TreeNode>(2);
    root3->right->right = std::make_unique<TreeNode>(3);
    runTestCase("Test Case 3: Right-Skewed Tree", root3);
    
    // Test Case 4: A single node tree
    auto root4 = std::make_unique<TreeNode>(5);
    runTestCase("Test Case 4: Single Node Tree", root4);

    // Test Case 5: An empty tree
    std::unique_ptr<TreeNode> root5 = nullptr;
    runTestCase("Test Case 5: Empty Tree", root5);

    return 0;
}