
#include <iostream>
#include <vector>
using namespace std;

class Node {
public:
    int data;
    Node* left;
    Node* right;
    
    Node(int value) {
        data = value;
        left = nullptr;
        right = nullptr;
    }
};

class BST {
private:
    Node* root;
    
    Node* insertHelper(Node* node, int value) {
        if (node == nullptr) {
            return new Node(value);
        }
        
        if (value < node->data) {
            node->left = insertHelper(node->left, value);
        } else if (value > node->data) {
            node->right = insertHelper(node->right, value);
        }
        
        return node;
    }
    
    Node* searchHelper(Node* node, int value) {
        if (node == nullptr || node->data == value) {
            return node;
        }
        
        if (value < node->data) {
            return searchHelper(node->left, value);
        }
        
        return searchHelper(node->right, value);
    }
    
    Node* findMin(Node* node) {
        while (node->left != nullptr) {
            node = node->left;
        }
        return node;
    }
    
    Node* deleteHelper(Node* node, int value) {
        if (node == nullptr) {
            return nullptr;
        }
        
        if (value < node->data) {
            node->left = deleteHelper(node->left, value);
        } else if (value > node->data) {
            node->right = deleteHelper(node->right, value);
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
            
            Node* minNode = findMin(node->right);
            node->data = minNode->data;
            node->right = deleteHelper(node->right, minNode->data);
        }
        
        return node;
    }
    
    void inorderHelper(Node* node, vector<int>& result) {
        if (node != nullptr) {
            inorderHelper(node->left, result);
            result.push_back(node->data);
            inorderHelper(node->right, result);
        }
    }
    
public:
    BST() {
        root = nullptr;
    }
    
    void insert(int value) {
        root = insertHelper(root, value);
    }
    
    bool search(int value) {
        return searchHelper(root, value) != nullptr;
    }
    
    void deleteNode(int value) {
        root = deleteHelper(root, value);
    }
    
    vector<int> inorder() {
        vector<int> result;
        inorderHelper(root, result);
        return result;
    }
};

int main() {
    // Test Case 1: Basic insert and search
    cout << "Test Case 1: Basic insert and search" << endl;
    BST bst1;
    bst1.insert(50);
    bst1.insert(30);
    bst1.insert(70);
    bst1.insert(20);
    bst1.insert(40);
    cout << "Inorder: ";
    vector<int> result1 = bst1.inorder();
    for (int val : result1) cout << val << " ";
    cout << endl;
    cout << "Search 40: " << (bst1.search(40) ? "true" : "false") << endl;
    cout << "Search 60: " << (bst1.search(60) ? "true" : "false") << endl;
    cout << endl;
    
    // Test Case 2: Delete leaf node
    cout << "Test Case 2: Delete leaf node" << endl;
    BST bst2;
    bst2.insert(50);
    bst2.insert(30);
    bst2.insert(70);
    bst2.insert(20);
    cout << "Before delete: ";
    vector<int> result2 = bst2.inorder();
    for (int val : result2) cout << val << " ";
    cout << endl;
    bst2.deleteNode(20);
    cout << "After delete 20: ";
    vector<int> result2b = bst2.inorder();
    for (int val : result2b) cout << val << " ";
    cout << endl << endl;
    
    // Test Case 3: Delete node with one child
    cout << "Test Case 3: Delete node with one child" << endl;
    BST bst3;
    bst3.insert(50);
    bst3.insert(30);
    bst3.insert(70);
    bst3.insert(60);
    cout << "Before delete: ";
    vector<int> result3 = bst3.inorder();
    for (int val : result3) cout << val << " ";
    cout << endl;
    bst3.deleteNode(70);
    cout << "After delete 70: ";
    vector<int> result3b = bst3.inorder();
    for (int val : result3b) cout << val << " ";
    cout << endl << endl;
    
    // Test Case 4: Delete node with two children
    cout << "Test Case 4: Delete node with two children" << endl;
    BST bst4;
    bst4.insert(50);
    bst4.insert(30);
    bst4.insert(70);
    bst4.insert(20);
    bst4.insert(40);
    bst4.insert(60);
    bst4.insert(80);
    cout << "Before delete: ";
    vector<int> result4 = bst4.inorder();
    for (int val : result4) cout << val << " ";
    cout << endl;
    bst4.deleteNode(50);
    cout << "After delete 50: ";
    vector<int> result4b = bst4.inorder();
    for (int val : result4b) cout << val << " ";
    cout << endl << endl;
    
    // Test Case 5: Multiple operations
    cout << "Test Case 5: Multiple operations" << endl;
    BST bst5;
    bst5.insert(15);
    bst5.insert(10);
    bst5.insert(20);
    bst5.insert(8);
    bst5.insert(12);
    bst5.insert(17);
    bst5.insert(25);
    cout << "Initial tree: ";
    vector<int> result5 = bst5.inorder();
    for (int val : result5) cout << val << " ";
    cout << endl;
    cout << "Search 12: " << (bst5.search(12) ? "true" : "false") << endl;
    bst5.deleteNode(15);
    cout << "After delete 15: ";
    vector<int> result5b = bst5.inorder();
    for (int val : result5b) cout << val << " ";
    cout << endl;
    bst5.insert(18);
    cout << "After insert 18: ";
    vector<int> result5c = bst5.inorder();
    for (int val : result5c) cout << val << " ";
    cout << endl;
    
    return 0;
}
