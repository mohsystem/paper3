#include <iostream>

// Node structure for the binary tree
struct Node {
    int data;
    Node* left;
    Node* right;
};

// Utility function to create a new node
Node* createNode(int data) {
    try {
        Node* newNode = new Node();
        newNode->data = data;
        newNode->left = nullptr;
        newNode->right = nullptr;
        return newNode;
    } catch (const std::bad_alloc& e) {
        std::cerr << "Memory allocation failed: " << e.what() << std::endl;
        return nullptr;
    }
}

// Utility function to free the tree memory to prevent leaks
void freeTree(Node* node) {
    if (node == nullptr) {
        return;
    }
    freeTree(node->left);
    freeTree(node->right);
    delete node;
}

/**
 * Performs a preorder traversal of the binary tree.
 * Root -> Left -> Right
 * @param node The root node of the tree/subtree.
 */
void preorderTraversal(Node* node) {
    if (node == nullptr) {
        return;
    }
    std::cout << node->data << " ";
    preorderTraversal(node->left);
    preorderTraversal(node->right);
}

/**
 * Performs an inorder traversal of the binary tree.
 * Left -> Root -> Right
 * @param node The root node of the tree/subtree.
 */
void inorderTraversal(Node* node) {
    if (node == nullptr) {
        return;
    }
    inorderTraversal(node->left);
    std::cout << node->data << " ";
    inorderTraversal(node->right);
}

/**
 * Performs a postorder traversal of the binary tree.
 * Left -> Right -> Root
 * @param node The root node of the tree/subtree.
 */
void postorderTraversal(Node* node) {
    if (node == nullptr) {
        return;
    }
    postorderTraversal(node->left);
    postorderTraversal(node->right);
    std::cout << node->data << " ";
}


int main() {
    // Test Case 1: A complete binary tree
    std::cout << "--- Test Case 1: Complete Binary Tree ---" << std::endl;
    Node* root1 = createNode(1);
    if(root1) {
        root1->left = createNode(2);
        root1->right = createNode(3);
        if(root1->left) {
            root1->left->left = createNode(4);
            root1->left->right = createNode(5);
        }
        if(root1->right) {
            root1->right->left = createNode(6);
            root1->right->right = createNode(7);
        }
    }
    std::cout << "Preorder:  ";
    preorderTraversal(root1);
    std::cout << std::endl;
    std::cout << "Inorder:   ";
    inorderTraversal(root1);
    std::cout << std::endl;
    std::cout << "Postorder: ";
    postorderTraversal(root1);
    std::cout << std::endl << std::endl;
    freeTree(root1);

    // Test Case 2: A right-skewed tree
    std::cout << "--- Test Case 2: Right-Skewed Tree ---" << std::endl;
    Node* root2 = createNode(10);
    if(root2) {
        root2->right = createNode(20);
        if(root2->right) {
            root2->right->right = createNode(30);
            if(root2->right->right) {
                root2->right->right->right = createNode(40);
            }
        }
    }
    std::cout << "Preorder:  ";
    preorderTraversal(root2);
    std::cout << std::endl;
    std::cout << "Inorder:   ";
    inorderTraversal(root2);
    std::cout << std::endl;
    std::cout << "Postorder: ";
    postorderTraversal(root2);
    std::cout << std::endl << std::endl;
    freeTree(root2);
    
    // Test Case 3: A left-skewed tree
    std::cout << "--- Test Case 3: Left-Skewed Tree ---" << std::endl;
    Node* root3 = createNode(50);
    if(root3) {
        root3->left = createNode(40);
        if(root3->left) {
            root3->left->left = createNode(30);
            if(root3->left->left) {
                root3->left->left->left = createNode(20);
            }
        }
    }
    std::cout << "Preorder:  ";
    preorderTraversal(root3);
    std::cout << std::endl;
    std::cout << "Inorder:   ";
    inorderTraversal(root3);
    std::cout << std::endl;
    std::cout << "Postorder: ";
    postorderTraversal(root3);
    std::cout << std::endl << std::endl;
    freeTree(root3);

    // Test Case 4: An empty tree (nullptr root)
    std::cout << "--- Test Case 4: Empty Tree ---" << std::endl;
    Node* root4 = nullptr;
    std::cout << "Preorder:  " << "(empty)" << std::endl;
    std::cout << "Inorder:   " << "(empty)" << std::endl;
    std::cout << "Postorder: " << "(empty)" << std::endl << std::endl;
    freeTree(root4);

    // Test Case 5: A tree with only one node
    std::cout << "--- Test Case 5: Single Node Tree ---" << std::endl;
    Node* root5 = createNode(100);
    std::cout << "Preorder:  ";
    preorderTraversal(root5);
    std::cout << std::endl;
    std::cout << "Inorder:   ";
    inorderTraversal(root5);
    std::cout << std::endl;
    std::cout << "Postorder: ";
    postorderTraversal(root5);
    std::cout << std::endl << std::endl;
    freeTree(root5);

    return 0;
}