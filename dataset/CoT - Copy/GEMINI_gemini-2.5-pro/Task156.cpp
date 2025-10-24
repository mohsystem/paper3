#include <iostream>

struct Node {
    int data;
    Node* left;
    Node* right;

    // Constructor to initialize a node
    Node(int val) : data(val), left(nullptr), right(nullptr) {}
};

// Preorder traversal: Root -> Left -> Right
void preorderTraversal(Node* node) {
    if (node == nullptr) {
        return;
    }
    std::cout << node->data << " ";
    preorderTraversal(node->left);
    preorderTraversal(node->right);
}

// Inorder traversal: Left -> Root -> Right
void inorderTraversal(Node* node) {
    if (node == nullptr) {
        return;
    }
    inorderTraversal(node->left);
    std::cout << node->data << " ";
    inorderTraversal(node->right);
}

// Postorder traversal: Left -> Right -> Root
void postorderTraversal(Node* node) {
    if (node == nullptr) {
        return;
    }
    postorderTraversal(node->left);
    postorderTraversal(node->right);
    std::cout << node->data << " ";
}

// Function to delete the entire tree to prevent memory leaks
void deleteTree(Node* node) {
    if (node == nullptr) {
        return;
    }
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

int main() {
    // --- Test Case 1: A standard binary tree ---
    std::cout << "--- Test Case 1: Standard Tree ---" << std::endl;
    Node* root1 = new Node(4);
    root1->left = new Node(2);
    root1->right = new Node(5);
    root1->left->left = new Node(1);
    root1->left->right = new Node(3);
    
    std::cout << "Preorder: "; preorderTraversal(root1); std::cout << std::endl;
    std::cout << "Inorder:  "; inorderTraversal(root1); std::cout << std::endl;
    std::cout << "Postorder:"; postorderTraversal(root1); std::cout << std::endl << std::endl;
    deleteTree(root1);

    // --- Test Case 2: A right-skewed tree ---
    std::cout << "--- Test Case 2: Right-Skewed Tree ---" << std::endl;
    Node* root2 = new Node(1);
    root2->right = new Node(2);
    root2->right->right = new Node(3);
    
    std::cout << "Preorder: "; preorderTraversal(root2); std::cout << std::endl;
    std::cout << "Inorder:  "; inorderTraversal(root2); std::cout << std::endl;
    std::cout << "Postorder:"; postorderTraversal(root2); std::cout << std::endl << std::endl;
    deleteTree(root2);

    // --- Test Case 3: A left-skewed tree ---
    std::cout << "--- Test Case 3: Left-Skewed Tree ---" << std::endl;
    Node* root3 = new Node(3);
    root3->left = new Node(2);
    root3->left->left = new Node(1);
    
    std::cout << "Preorder: "; preorderTraversal(root3); std::cout << std::endl;
    std::cout << "Inorder:  "; inorderTraversal(root3); std::cout << std::endl;
    std::cout << "Postorder:"; postorderTraversal(root3); std::cout << std::endl << std::endl;
    deleteTree(root3);
    
    // --- Test Case 4: A tree with only a root node ---
    std::cout << "--- Test Case 4: Single Node Tree ---" << std::endl;
    Node* root4 = new Node(10);
    
    std::cout << "Preorder: "; preorderTraversal(root4); std::cout << std::endl;
    std::cout << "Inorder:  "; inorderTraversal(root4); std::cout << std::endl;
    std::cout << "Postorder:"; postorderTraversal(root4); std::cout << std::endl << std::endl;
    deleteTree(root4);

    // --- Test Case 5: An empty tree (nullptr root) ---
    std::cout << "--- Test Case 5: Empty Tree ---" << std::endl;
    Node* root5 = nullptr;
    
    std::cout << "Preorder: "; preorderTraversal(root5); std::cout << std::endl;
    std::cout << "Inorder:  "; inorderTraversal(root5); std::cout << std::endl;
    std::cout << "Postorder:"; postorderTraversal(root5); std::cout << std::endl;

    return 0;
}