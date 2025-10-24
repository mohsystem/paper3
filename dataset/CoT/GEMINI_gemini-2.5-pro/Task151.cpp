#include <iostream>

struct Node {
    int key;
    Node *left;
    Node *right;

    Node(int item) {
        key = item;
        left = right = nullptr;
    }
};

class Task151 {
private:
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
    
    Node* minValueNode(Node* node) {
        Node* current = node;
        while (current && current->left != nullptr) {
            current = current->left;
        }
        return current;
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
            Node* temp = minValueNode(node->right);
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
        if (node != nullptr) {
            destroyTree(node->left);
            destroyTree(node->right);
            delete node;
        }
    }

public:
    Task151() : root(nullptr) {}

    ~Task151() {
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
    // --- Test Case 1: Insertion and Search ---
    std::cout << "--- Test Case 1: Insertion and Search ---" << std::endl;
    Task151 tree;
    tree.insert(50);
    tree.insert(30);
    tree.insert(20);
    tree.insert(40);
    tree.insert(70);
    tree.insert(60);
    tree.insert(80);
    std::cout << "Inorder traversal: ";
    tree.inorder();
    std::cout << "Search for 60: " << (tree.search(60) ? "Found" : "Not Found") << std::endl;
    std::cout << "Search for 90: " << (tree.search(90) ? "Found" : "Not Found") << std::endl;
    std::cout << std::endl;

    // --- Test Case 2: Deleting a leaf node (20) ---
    std::cout << "--- Test Case 2: Deleting a leaf node (20) ---" << std::endl;
    std::cout << "Deleting 20..." << std::endl;
    tree.deleteNode(20);
    std::cout << "Inorder traversal: ";
    tree.inorder();
    std::cout << std::endl;

    // --- Test Case 3: Deleting a node with one child (30) ---
    std::cout << "--- Test Case 3: Deleting a node with one child (30) ---" << std::endl;
    std::cout << "Deleting 30..." << std::endl;
    tree.deleteNode(30);
    std::cout << "Inorder traversal: ";
    tree.inorder();
    std::cout << std::endl;

    // --- Test Case 4: Deleting a node with two children (50) ---
    std::cout << "--- Test Case 4: Deleting a node with two children (50) ---" << std::endl;
    std::cout << "Deleting 50 (root)..." << std::endl;
    tree.deleteNode(50);
    std::cout << "Inorder traversal: ";
    tree.inorder();
    std::cout << std::endl;

    // --- Test Case 5: Deleting another node and searching for a deleted key ---
    std::cout << "--- Test Case 5: Deleting another node and checking ---" << std::endl;
    std::cout << "Deleting 70..." << std::endl;
    tree.deleteNode(70);
    std::cout << "Inorder traversal: ";
    tree.inorder();
    std::cout << "Search for 50: " << (tree.search(50) ? "Found" : "Not Found") << std::endl;

    return 0;
}