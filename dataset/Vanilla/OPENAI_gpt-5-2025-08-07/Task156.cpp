#include <iostream>
#include <vector>
#include <string>

struct TreeNode {
    int val;
    TreeNode* left;
    TreeNode* right;
    TreeNode(int v) : val(v), left(nullptr), right(nullptr) {}
    TreeNode(int v, TreeNode* l, TreeNode* r) : val(v), left(l), right(r) {}
};

void preorderHelper(TreeNode* node, std::vector<int>& res) {
    if (!node) return;
    res.push_back(node->val);
    preorderHelper(node->left, res);
    preorderHelper(node->right, res);
}
void inorderHelper(TreeNode* node, std::vector<int>& res) {
    if (!node) return;
    inorderHelper(node->left, res);
    res.push_back(node->val);
    inorderHelper(node->right, res);
}
void postorderHelper(TreeNode* node, std::vector<int>& res) {
    if (!node) return;
    postorderHelper(node->left, res);
    postorderHelper(node->right, res);
    res.push_back(node->val);
}

std::vector<int> preorder(TreeNode* root) {
    std::vector<int> res;
    preorderHelper(root, res);
    return res;
}
std::vector<int> inorder(TreeNode* root) {
    std::vector<int> res;
    inorderHelper(root, res);
    return res;
}
std::vector<int> postorder(TreeNode* root) {
    std::vector<int> res;
    postorderHelper(root, res);
    return res;
}

// Public API: performs DFS traversal based on order: "preorder", "inorder", "postorder"
std::vector<int> dfs(TreeNode* root, const std::string& order) {
    std::string o = order;
    for (auto& c : o) c = std::tolower(c);
    if (o == "preorder") return preorder(root);
    if (o == "inorder") return inorder(root);
    if (o == "postorder") return postorder(root);
    throw std::invalid_argument("Unknown order: " + order);
}

void printVec(const std::string& label, const std::vector<int>& v) {
    std::cout << label << ": [";
    for (size_t i = 0; i < v.size(); ++i) {
        std::cout << v[i];
        if (i + 1 < v.size()) std::cout << ", ";
    }
    std::cout << "]\n";
}

void freeTree(TreeNode* node) {
    if (!node) return;
    freeTree(node->left);
    freeTree(node->right);
    delete node;
}

int main() {
    // Test 1: Balanced tree
    TreeNode* root1 = new TreeNode(1,
        new TreeNode(2, new TreeNode(4), new TreeNode(5)),
        new TreeNode(3, new TreeNode(6), new TreeNode(7))
    );
    printVec("Test1 Preorder", dfs(root1, "preorder")); // [1,2,4,5,3,6,7]
    freeTree(root1);

    // Test 2: Single node
    TreeNode* root2 = new TreeNode(10);
    printVec("Test2 Inorder", dfs(root2, "inorder")); // [10]
    freeTree(root2);

    // Test 3: Left-skewed
    TreeNode* root3 = new TreeNode(5);
    root3->left = new TreeNode(4);
    root3->left->left = new TreeNode(3);
    root3->left->left->left = new TreeNode(2);
    root3->left->left->left->left = new TreeNode(1);
    printVec("Test3 Postorder", dfs(root3, "postorder")); // [1,2,3,4,5]
    freeTree(root3);

    // Test 4: Right-skewed
    TreeNode* root4 = new TreeNode(1);
    root4->right = new TreeNode(2);
    root4->right->right = new TreeNode(3);
    root4->right->right->right = new TreeNode(4);
    root4->right->right->right->right = new TreeNode(5);
    printVec("Test4 Preorder", dfs(root4, "preorder")); // [1,2,3,4,5]
    freeTree(root4);

    // Test 5: Empty tree
    TreeNode* root5 = nullptr;
    printVec("Test5 Inorder (empty)", dfs(root5, "inorder")); // []
    return 0;
}