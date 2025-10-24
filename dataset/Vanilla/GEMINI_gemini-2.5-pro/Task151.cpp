#include <iostream>

class BST {
private:
    struct Node {
        int key;
        Node* left;
        Node* right;
        Node(int k) : key(k), left(nullptr), right(nullptr) {}
    };

    Node* root;

    Node* insertRec(Node* node, int key) {
        if (node == nullptr) {
            return new Node(key);
        }
        if (key < node->key) {
            node->left = insertRec(node->left, key);
        } else if (key > node->key) {
            node->right = insertRec(node->right, key);
        }
        return node;
    }

    Node* searchRec(Node* node, int key) {
        if (node == nullptr || node->key == key) {
            return node;
        }
        if (key < node->key) {
            return searchRec(node->left, key);
        }
        return searchRec(node->right, key);
    }

    Node* findMin(Node* node) {
        while (node && node->left != nullptr) {
            node = node->left;
        }
        return node;
    }

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

    void inorderRec(Node* node) {
        if (node != nullptr) {
            inorderRec(node->left);
            std::cout << node->key << " ";
            inorderRec(node->right);
        }
    }
    
    void destroyTree(Node* node) {
        if (node) {
            destroyTree(node->left);
            destroyTree(node->right);
            delete node;
        }
    }

public:
    BST() : root(nullptr) {}
    
    ~BST() {
        destroyTree(root);
    }

    void insert(int key) {
        root = insertRec(root, key);
    }

    bool search(int key) {
        return searchRec(root, key) != nullptr;
    }

    void deleteNode(int key) {
        root = deleteRec(root, key);
    }

    void inorder() {
        inorderRec(root);
        std::cout << std::endl;
    }
};

int main() {
    BST bst;

    // Test Case 1: Insertion
    std::cout << "--- Test Case 1: Insertion ---" << std::endl;
    int keysToInsert[] = {50, 30, 20, 40, 70, 60, 80};
    for (int key : keysToInsert) {
        bst.insert(key);
    }
    std::cout << "Inorder traversal of the initial BST: ";
    bst.inorder();

    // Test Case 2: Search
    std::cout << "\n--- Test Case 2: Search ---" << std::endl;
    std::cout << "Search for 60: " << (bst.search(60) ? "Found" : "Not Found") << std::endl;
    std::cout << "Search for 90: " << (bst.search(90) ? "Found" : "Not Found") << std::endl;

    // Test Case 3: Delete a leaf node
    std::cout << "\n--- Test Case 3: Delete a leaf node (20) ---" << std::endl;
    bst.deleteNode(20);
    std::cout << "Inorder traversal after deleting 20: ";
    bst.inorder();

    // Test Case 4: Delete a node with one child
    std::cout << "\n--- Test Case 4: Delete a node with one child (30) ---" << std::endl;
    bst.deleteNode(30);
    std::cout << "Inorder traversal after deleting 30: ";
    bst.inorder();

    // Test Case 5: Delete a node with two children
    std::cout << "\n--- Test Case 5: Delete a node with two children (50) ---" << std::endl;
    bst.deleteNode(50);
    std::cout << "Inorder traversal after deleting 50: ";
    bst.inorder();

    return 0;
}