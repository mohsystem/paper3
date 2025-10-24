
#include <iostream>
#include <vector>
#include <memory>
#include <stdexcept>
#include <limits>

// Node structure for binary tree
struct TreeNode {
    int value;
    std::unique_ptr<TreeNode> left;
    std::unique_ptr<TreeNode> right;
    
    explicit TreeNode(int val) : value(val), left(nullptr), right(nullptr) {}
};

// Binary tree class with depth-first traversal methods
class BinaryTree {
private:
    std::unique_ptr<TreeNode> root;
    
    // Helper function for preorder traversal
    // Validates recursion depth to prevent stack overflow
    void preorderHelper(TreeNode* node, std::vector<int>& result, int depth) {
        // Prevent excessive recursion depth (security: stack overflow protection)
        const int MAX_DEPTH = 10000;
        if (depth > MAX_DEPTH) {
            throw std::runtime_error("Maximum tree depth exceeded");
        }
        
        if (node == nullptr) {
            return;
        }
        
        // Check result size to prevent excessive memory usage
        const size_t MAX_NODES = 1000000;
        if (result.size() >= MAX_NODES) {
            throw std::runtime_error("Maximum node count exceeded");
        }
        
        result.push_back(node->value); // Visit root
        preorderHelper(node->left.get(), result, depth + 1);  // Visit left subtree
        preorderHelper(node->right.get(), result, depth + 1); // Visit right subtree
    }
    
    // Helper function for inorder traversal
    void inorderHelper(TreeNode* node, std::vector<int>& result, int depth) {
        const int MAX_DEPTH = 10000;
        if (depth > MAX_DEPTH) {
            throw std::runtime_error("Maximum tree depth exceeded");
        }
        
        if (node == nullptr) {
            return;
        }
        
        const size_t MAX_NODES = 1000000;
        if (result.size() >= MAX_NODES) {
            throw std::runtime_error("Maximum node count exceeded");
        }
        
        inorderHelper(node->left.get(), result, depth + 1);   // Visit left subtree
        result.push_back(node->value);                         // Visit root
        inorderHelper(node->right.get(), result, depth + 1);  // Visit right subtree
    }
    
    // Helper function for postorder traversal
    void postorderHelper(TreeNode* node, std::vector<int>& result, int depth) {
        const int MAX_DEPTH = 10000;
        if (depth > MAX_DEPTH) {
            throw std::runtime_error("Maximum tree depth exceeded");
        }
        
        if (node == nullptr) {
            return;
        }
        
        const size_t MAX_NODES = 1000000;
        if (result.size() >= MAX_NODES) {
            throw std::runtime_error("Maximum node count exceeded");
        }
        
        postorderHelper(node->left.get(), result, depth + 1);  // Visit left subtree
        postorderHelper(node->right.get(), result, depth + 1); // Visit right subtree
        result.push_back(node->value);                          // Visit root
    }
    
public:
    BinaryTree() : root(nullptr) {}
    
    // Insert node at root (for testing purposes)
    void setRoot(int value) {
        root = std::make_unique<TreeNode>(value);
    }
    
    // Insert left child - validates parent exists
    void insertLeft(TreeNode* parent, int value) {
        if (parent == nullptr) {
            throw std::invalid_argument("Parent node cannot be null");
        }
        parent->left = std::make_unique<TreeNode>(value);
    }
    
    // Insert right child - validates parent exists
    void insertRight(TreeNode* parent, int value) {
        if (parent == nullptr) {
            throw std::invalid_argument("Parent node cannot be null");
        }
        parent->right = std::make_unique<TreeNode>(value);
    }
    
    TreeNode* getRoot() const {
        return root.get();
    }
    
    // Public preorder traversal method
    std::vector<int> preorder() {
        std::vector<int> result;
        try {
            preorderHelper(root.get(), result, 0);
        } catch (const std::exception& e) {
            std::cerr << "Error during preorder traversal: " << e.what() << std::endl;
            throw;
        }
        return result;
    }
    
    // Public inorder traversal method
    std::vector<int> inorder() {
        std::vector<int> result;
        try {
            inorderHelper(root.get(), result, 0);
        } catch (const std::exception& e) {
            std::cerr << "Error during inorder traversal: " << e.what() << std::endl;
            throw;
        }
        return result;
    }
    
    // Public postorder traversal method
    std::vector<int> postorder() {
        std::vector<int> result;
        try {
            postorderHelper(root.get(), result, 0);
        } catch (const std::exception& e) {
            std::cerr << "Error during postorder traversal: " << e.what() << std::endl;
            throw;
        }
        return result;
    }
};

// Helper function to print vector
void printVector(const std::vector<int>& vec) {
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i];
        if (i < vec.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;
}

int main() {
    try {
        // Test case 1: Simple tree with 3 nodes
        std::cout << "Test Case 1: Simple tree (1, 2, 3)" << std::endl;
        BinaryTree tree1;
        tree1.setRoot(1);
        tree1.insertLeft(tree1.getRoot(), 2);
        tree1.insertRight(tree1.getRoot(), 3);
        
        std::cout << "Preorder:  ";
        printVector(tree1.preorder());
        std::cout << "Inorder:   ";
        printVector(tree1.inorder());
        std::cout << "Postorder: ";
        printVector(tree1.postorder());
        std::cout << std::endl;
        
        // Test case 2: Balanced tree
        std::cout << "Test Case 2: Balanced tree" << std::endl;
        BinaryTree tree2;
        tree2.setRoot(4);
        tree2.insertLeft(tree2.getRoot(), 2);
        tree2.insertRight(tree2.getRoot(), 6);
        tree2.insertLeft(tree2.getRoot()->left.get(), 1);
        tree2.insertRight(tree2.getRoot()->left.get(), 3);
        tree2.insertLeft(tree2.getRoot()->right.get(), 5);
        tree2.insertRight(tree2.getRoot()->right.get(), 7);
        
        std::cout << "Preorder:  ";
        printVector(tree2.preorder());
        std::cout << "Inorder:   ";
        printVector(tree2.inorder());
        std::cout << "Postorder: ";
        printVector(tree2.postorder());
        std::cout << std::endl;
        
        // Test case 3: Left-skewed tree
        std::cout << "Test Case 3: Left-skewed tree" << std::endl;
        BinaryTree tree3;
        tree3.setRoot(5);
        tree3.insertLeft(tree3.getRoot(), 4);
        tree3.insertLeft(tree3.getRoot()->left.get(), 3);
        
        std::cout << "Preorder:  ";
        printVector(tree3.preorder());
        std::cout << "Inorder:   ";
        printVector(tree3.inorder());
        std::cout << "Postorder: ";
        printVector(tree3.postorder());
        std::cout << std::endl;
        
        // Test case 4: Right-skewed tree
        std::cout << "Test Case 4: Right-skewed tree" << std::endl;
        BinaryTree tree4;
        tree4.setRoot(1);
        tree4.insertRight(tree4.getRoot(), 2);
        tree4.insertRight(tree4.getRoot()->right.get(), 3);
        
        std::cout << "Preorder:  ";
        printVector(tree4.preorder());
        std::cout << "Inorder:   ";
        printVector(tree4.inorder());
        std::cout << "Postorder: ";
        printVector(tree4.postorder());
        std::cout << std::endl;
        
        // Test case 5: Single node tree
        std::cout << "Test Case 5: Single node tree" << std::endl;
        BinaryTree tree5;
        tree5.setRoot(42);
        
        std::cout << "Preorder:  ";
        printVector(tree5.preorder());
        std::cout << "Inorder:   ";
        printVector(tree5.inorder());
        std::cout << "Postorder: ";
        printVector(tree5.postorder());
        
    } catch (const std::exception& e) {
        std::cerr << "Exception caught in main: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
