
#include <iostream>
#include <vector>
using namespace std;

class Node {
public:
    int data;
    Node* left;
    Node* right;
    
    Node(int data) {
        this->data = data;
        this->left = nullptr;
        this->right = nullptr;
    }
};

class BinarySearchTree {
private:
    Node* root;
    
    Node* insertHelper(Node* root, int data) {
        if (root == nullptr) {
            return new Node(data);
        }
        
        if (data < root->data) {
            root->left = insertHelper(root->left, data);
        } else if (data > root->data) {
            root->right = insertHelper(root->right, data);
        }
        
        return root;
    }
    
    bool searchHelper(Node* root, int data) {
        if (root == nullptr) {
            return false;
        }
        
        if (root->data == data) {
            return true;
        }
        
        if (data < root->data) {
            return searchHelper(root->left, data);
        } else {
            return searchHelper(root->right, data);
        }
    }
    
    Node* deleteHelper(Node* root, int data) {
        if (root == nullptr) {
            return nullptr;
        }
        
        if (data < root->data) {
            root->left = deleteHelper(root->left, data);
        } else if (data > root->data) {
            root->right = deleteHelper(root->right, data);
        } else {
            // Node to be deleted found
            
            // Case 1: No child or one child
            if (root->left == nullptr) {
                Node* temp = root->right;
                delete root;
                return temp;
            } else if (root->right == nullptr) {
                Node* temp = root->left;
                delete root;
                return temp;
            }
            
            // Case 2: Two children
            // Find inorder successor (smallest in right subtree)
            Node* successor = findMin(root->right);
            root->data = successor->data;
            root->right = deleteHelper(root->right, successor->data);
        }
        
        return root;
    }
    
    Node* findMin(Node* root) {
        while (root->left != nullptr) {
            root = root->left;
        }
        return root;
    }
    
    void inorderHelper(Node* root, vector<int>& result) {
        if (root != nullptr) {
            inorderHelper(root->left, result);
            result.push_back(root->data);
            inorderHelper(root->right, result);
        }
    }
    
public:
    BinarySearchTree() {
        root = nullptr;
    }
    
    void insert(int data) {
        root = insertHelper(root, data);
    }
    
    bool search(int data) {
        return searchHelper(root, data);
    }
    
    void deleteNode(int data) {
        root = deleteHelper(root, data);
    }
    
    vector<int> inorder() {
        vector<int> result;
        inorderHelper(root, result);
        return result;
    }
};

void printVector(const vector<int>& vec) {
    cout << "[";
    for (size_t i = 0; i < vec.size(); i++) {
        cout << vec[i];
        if (i < vec.size() - 1) cout << ", ";
    }
    cout << "]" << endl;
}

int main() {
    // Test Case 1: Basic insert and search
    cout << "Test Case 1: Basic insert and search" << endl;
    BinarySearchTree bst1;
    bst1.insert(50);
    bst1.insert(30);
    bst1.insert(70);
    bst1.insert(20);
    bst1.insert(40);
    cout << "Search 40: " << (bst1.search(40) ? "true" : "false") << endl;
    cout << "Search 60: " << (bst1.search(60) ? "true" : "false") << endl;
    cout << "Inorder: ";
    printVector(bst1.inorder());
    cout << endl;
    
    // Test Case 2: Delete leaf node
    cout << "Test Case 2: Delete leaf node" << endl;
    BinarySearchTree bst2;
    bst2.insert(50);
    bst2.insert(30);
    bst2.insert(70);
    bst2.insert(20);
    bst2.insert(40);
    bst2.deleteNode(20);
    cout << "After deleting 20: ";
    printVector(bst2.inorder());
    cout << "Search 20: " << (bst2.search(20) ? "true" : "false") << endl;
    cout << endl;
    
    // Test Case 3: Delete node with one child
    cout << "Test Case 3: Delete node with one child" << endl;
    BinarySearchTree bst3;
    bst3.insert(50);
    bst3.insert(30);
    bst3.insert(70);
    bst3.insert(60);
    bst3.deleteNode(70);
    cout << "After deleting 70: ";
    printVector(bst3.inorder());
    cout << endl;
    
    // Test Case 4: Delete node with two children
    cout << "Test Case 4: Delete node with two children" << endl;
    BinarySearchTree bst4;
    bst4.insert(50);
    bst4.insert(30);
    bst4.insert(70);
    bst4.insert(20);
    bst4.insert(40);
    bst4.insert(60);
    bst4.insert(80);
    bst4.deleteNode(50);
    cout << "After deleting 50: ";
    printVector(bst4.inorder());
    cout << endl;
    
    // Test Case 5: Complex operations
    cout << "Test Case 5: Complex operations" << endl;
    BinarySearchTree bst5;
    int values[] = {15, 10, 20, 8, 12, 17, 25};
    for (int val : values) {
        bst5.insert(val);
    }
    cout << "Initial tree: ";
    printVector(bst5.inorder());
    cout << "Search 12: " << (bst5.search(12) ? "true" : "false") << endl;
    bst5.deleteNode(15);
    cout << "After deleting 15: ";
    printVector(bst5.inorder());
    bst5.insert(14);
    cout << "After inserting 14: ";
    printVector(bst5.inorder());
    
    return 0;
}
