
#include <iostream>
#include <vector>
using namespace std;

class Task151 {
private:
    struct Node {
        int data;
        Node* left;
        Node* right;
        
        Node(int val) : data(val), left(nullptr), right(nullptr) {}
    };
    
    Node* root;
    
    Node* insertRec(Node* root, int data) {
        if (root == nullptr) {
            return new Node(data);
        }
        
        if (data < root->data) {
            root->left = insertRec(root->left, data);
        } else if (data > root->data) {
            root->right = insertRec(root->right, data);
        }
        
        return root;
    }
    
    bool searchRec(Node* root, int data) {
        if (root == nullptr) {
            return false;
        }
        
        if (root->data == data) {
            return true;
        }
        
        if (data < root->data) {
            return searchRec(root->left, data);
        } else {
            return searchRec(root->right, data);
        }
    }
    
    Node* deleteRec(Node* root, int data) {
        if (root == nullptr) {
            return nullptr;
        }
        
        if (data < root->data) {
            root->left = deleteRec(root->left, data);
        } else if (data > root->data) {
            root->right = deleteRec(root->right, data);
        } else {
            if (root->left == nullptr) {
                Node* temp = root->right;
                delete root;
                return temp;
            } else if (root->right == nullptr) {
                Node* temp = root->left;
                delete root;
                return temp;
            }
            
            root->data = minValue(root->right);
            root->right = deleteRec(root->right, root->data);
        }
        
        return root;
    }
    
    int minValue(Node* root) {
        int minVal = root->data;
        while (root->left != nullptr) {
            minVal = root->left->data;
            root = root->left;
        }
        return minVal;
    }
    
    void inorderRec(Node* root, vector<int>& result) {
        if (root != nullptr) {
            inorderRec(root->left, result);
            result.push_back(root->data);
            inorderRec(root->right, result);
        }
    }
    
    void destroyTree(Node* root) {
        if (root != nullptr) {
            destroyTree(root->left);
            destroyTree(root->right);
            delete root;
        }
    }
    
public:
    Task151() : root(nullptr) {}
    
    ~Task151() {
        destroyTree(root);
    }
    
    void insert(int data) {
        root = insertRec(root, data);
    }
    
    bool search(int data) {
        return searchRec(root, data);
    }
    
    void remove(int data) {
        root = deleteRec(root, data);
    }
    
    vector<int> inorder() {
        vector<int> result;
        inorderRec(root, result);
        return result;
    }
};

void printVector(const vector<int>& vec) {
    for (int val : vec) {
        cout << val << " ";
    }
    cout << endl;
}

int main() {
    // Test Case 1: Basic insertion and search
    cout << "Test Case 1: Basic insertion and search" << endl;
    Task151 bst1;
    bst1.insert(50);
    bst1.insert(30);
    bst1.insert(70);
    bst1.insert(20);
    bst1.insert(40);
    cout << "Inorder traversal: ";
    printVector(bst1.inorder());
    cout << "Search 40: " << (bst1.search(40) ? "true" : "false") << endl;
    cout << "Search 60: " << (bst1.search(60) ? "true" : "false") << endl;
    cout << endl;
    
    // Test Case 2: Delete leaf node
    cout << "Test Case 2: Delete leaf node" << endl;
    Task151 bst2;
    bst2.insert(50);
    bst2.insert(30);
    bst2.insert(70);
    bst2.insert(20);
    cout << "Before delete: ";
    printVector(bst2.inorder());
    bst2.remove(20);
    cout << "After delete 20: ";
    printVector(bst2.inorder());
    cout << endl;
    
    // Test Case 3: Delete node with one child
    cout << "Test Case 3: Delete node with one child" << endl;
    Task151 bst3;
    bst3.insert(50);
    bst3.insert(30);
    bst3.insert(70);
    bst3.insert(60);
    cout << "Before delete: ";
    printVector(bst3.inorder());
    bst3.remove(70);
    cout << "After delete 70: ";
    printVector(bst3.inorder());
    cout << endl;
    
    // Test Case 4: Delete node with two children
    cout << "Test Case 4: Delete node with two children" << endl;
    Task151 bst4;
    bst4.insert(50);
    bst4.insert(30);
    bst4.insert(70);
    bst4.insert(20);
    bst4.insert(40);
    bst4.insert(60);
    bst4.insert(80);
    cout << "Before delete: ";
    printVector(bst4.inorder());
    bst4.remove(50);
    cout << "After delete 50: ";
    printVector(bst4.inorder());
    cout << endl;
    
    // Test Case 5: Empty tree operations
    cout << "Test Case 5: Empty tree operations" << endl;
    Task151 bst5;
    cout << "Search in empty tree: " << (bst5.search(10) ? "true" : "false") << endl;
    bst5.remove(10);
    cout << "Inorder of empty tree: ";
    printVector(bst5.inorder());
    
    return 0;
}
