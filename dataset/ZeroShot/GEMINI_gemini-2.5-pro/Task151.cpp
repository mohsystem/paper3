#include <iostream>

struct Node {
    int key;
    Node *left;
    Node *right;

    // Use a constructor for safe initialization
    explicit Node(int k) : key(k), left(nullptr), right(nullptr) {}
};

class BinarySearchTree {
private:
    Node *root;

    // Private helper functions
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
        if (node == nullptr) {
           return nullptr;
        }
        Node* current = node;
        while (current && current->left != nullptr) {
            current = current->left;
        }
        return current;
    }

    Node* removeRec(Node* node, int key) {
        if (node == nullptr) {
            return node;
        }
        if (key < node->key) {
            node->left = removeRec(node->left, key);
        } else if (key > node->key) {
            node->right = removeRec(node->right, key);
        } else {
            // Node with only one child or no child
            if (node->left == nullptr) {
                Node* temp = node->right;
                delete node;
                return temp;
            } else if (node->right == nullptr) {
                Node* temp = node->left;
                delete node;
                return temp;
            }
            // Node with two children
            Node* temp = findMin(node->right);
            node->key = temp->key;
            node->right = removeRec(node->right, temp->key);
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
    BinarySearchTree() : root(nullptr) {}

    ~BinarySearchTree() {
        destroyTree(root);
    }

    void insert(int key) {
        root = insertRec(root, key);
    }

    bool search(int key) {
        return searchRec(root, key) != nullptr;
    }

    void remove(int key) {
        root = removeRec(root, key);
    }
    
    void inorder() {
        inorderRec(root);
        std::cout << std::endl;
    }
};

int main() {
    BinarySearchTree tree;
    /*
     *        50
     *       /  \
     *      30   70
     *     / \   / \
     *    20 40 60  80
     */
    tree.insert(50);
    tree.insert(30);
    tree.insert(20);
    tree.insert(40);
    tree.insert(70);
    tree.insert(60);
    tree.insert(80);

    std::cout << "Initial tree (inorder):" << std::endl;
    tree.inorder();

    // Test Case 1: Search for an element that exists
    std::cout << "Test Case 1: Search for 40" << std::endl;
    std::cout << "Found 40: " << (tree.search(40) ? "true" : "false") << std::endl;

    // Test Case 2: Search for an element that does not exist
    std::cout << "\nTest Case 2: Search for 90" << std::endl;
    std::cout << "Found 90: " << (tree.search(90) ? "true" : "false") << std::endl;
    
    // Test Case 3: Delete a leaf node (20)
    std::cout << "\nTest Case 3: Delete 20 (leaf node)" << std::endl;
    tree.remove(20);
    std::cout << "Tree after deleting 20: ";
    tree.inorder();

    // Test Case 4: Delete a node with one child (30)
    std::cout << "\nTest Case 4: Delete 30 (node with one child)" << std::endl;
    tree.remove(30);
    std::cout << "Tree after deleting 30: ";
    tree.inorder();
    
    // Test Case 5: Delete a node with two children (50 - the root)
    std::cout << "\nTest Case 5: Delete 50 (node with two children - root)" << std::endl;
    tree.remove(50);
    std::cout << "Tree after deleting 50: ";
    tree.inorder();

    return 0;
}