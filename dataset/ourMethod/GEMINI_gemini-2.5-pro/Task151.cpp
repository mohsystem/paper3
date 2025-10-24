#include <iostream>

// Node structure for the BST
struct Node {
    int key;
    Node *left;
    Node *right;

    // Constructor to initialize a new node
    Node(int k) : key(k), left(nullptr), right(nullptr) {}
};

class BinarySearchTree {
private:
    Node* root;

    // Recursive helper to insert a key
    Node* insertRec(Node* node, int key) {
        if (node == nullptr) {
            try {
                return new Node(key);
            } catch (const std::bad_alloc& e) {
                std::cerr << "Memory allocation failed: " << e.what() << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        if (key < node->key) {
            node->left = insertRec(node->left, key);
        } else if (key > node->key) {
            node->right = insertRec(node->right, key);
        }
        return node;
    }

    // Recursive helper to search for a key
    bool searchRec(Node* node, int key) const {
        if (node == nullptr) {
            return false;
        }
        if (node->key == key) {
            return true;
        }
        return key < node->key ? searchRec(node->left, key) : searchRec(node->right, key);
    }

    // Helper to find the minimum value node in a subtree
    Node* findMin(Node* node) {
        while (node && node->left != nullptr) {
            node = node->left;
        }
        return node;
    }

    // Recursive helper to delete a key
    Node* deleteRec(Node* node, int key) {
        if (node == nullptr) {
            return node;
        }

        if (key < node->key) {
            node->left = deleteRec(node->left, key);
        } else if (key > node->key) {
            node->right = deleteRec(node->right, key);
        } else {
            if (node->left == nullptr) {
                Node* temp = node->right;
                delete node;
                return temp;
            } else if (node->right == nullptr) {
                Node* temp = node->left;
                delete node;
                return temp;
            }

            Node* temp = findMin(node->right);
            node->key = temp->key;
            node->right = deleteRec(node->right, temp->key);
        }
        return node;
    }

    // Helper to destroy the tree and free memory
    void destroyTree(Node* node) {
        if (node) {
            destroyTree(node->left);
            destroyTree(node->right);
            delete node;
        }
    }

public:
    // Constructor
    BinarySearchTree() : root(nullptr) {}

    // Destructor to free all nodes
    ~BinarySearchTree() {
        destroyTree(root);
    }

    // Public method to insert a key
    void insert(int key) {
        root = insertRec(root, key);
    }

    // Public method to delete a key
    void remove(int key) {
        root = deleteRec(root, key);
    }

    // Public method to search for a key
    bool search(int key) const {
        return searchRec(root, key);
    }
};

int main() {
    BinarySearchTree bst;

    // Test Case 1: Insert and search
    std::cout << "Test Case 1: Insert and Search" << std::endl;
    bst.insert(50);
    bst.insert(30);
    bst.insert(70);
    std::cout << "Search for 30: " << (bst.search(30) ? "true" : "false") << std::endl;
    std::cout << "Search for 100: " << (bst.search(100) ? "true" : "false") << std::endl;
    std::cout << "---" << std::endl;

    // Test Case 2: Delete a leaf node
    std::cout << "Test Case 2: Delete a leaf node" << std::endl;
    bst.insert(20);
    std::cout << "Search for 20 before delete: " << (bst.search(20) ? "true" : "false") << std::endl;
    bst.remove(20);
    std::cout << "Search for 20 after delete: " << (bst.search(20) ? "true" : "false") << std::endl;
    std::cout << "---" << std::endl;

    // Test Case 3: Delete a node with one child
    std::cout << "Test Case 3: Delete a node with one child" << std::endl;
    bst.insert(40);
    std::cout << "Search for 30 before delete: " << (bst.search(30) ? "true" : "false") << std::endl;
    std::cout << "Search for 40 before delete: " << (bst.search(40) ? "true" : "false") << std::endl;
    bst.remove(30);
    std::cout << "Search for 30 after delete: " << (bst.search(30) ? "true" : "false") << std::endl;
    std::cout << "Search for 40 after delete: " << (bst.search(40) ? "true" : "false") << std::endl;
    std::cout << "---" << std::endl;

    // Test Case 4: Delete a node with two children
    std::cout << "Test Case 4: Delete a node with two children" << std::endl;
    bst.insert(60);
    bst.insert(80);
    std::cout << "Search for 70 before delete: " << (bst.search(70) ? "true" : "false") << std::endl;
    bst.remove(70);
    std::cout << "Search for 70 after delete: " << (bst.search(70) ? "true" : "false") << std::endl;
    std::cout << "Search for 80 (new subtree root): " << (bst.search(80) ? "true" : "false") << std::endl;
    std::cout << "---" << std::endl;

    // Test Case 5: Delete root node
    std::cout << "Test Case 5: Delete root node" << std::endl;
    std::cout << "Search for 50 before delete: " << (bst.search(50) ? "true" : "false") << std::endl;
    bst.remove(50);
    std::cout << "Search for 50 after delete: " << (bst.search(50) ? "true" : "false") << std::endl;
    std::cout << "Search for 60 (new root): " << (bst.search(60) ? "true" : "false") << std::endl;
    std::cout << "---" << std::endl;

    return 0;
}