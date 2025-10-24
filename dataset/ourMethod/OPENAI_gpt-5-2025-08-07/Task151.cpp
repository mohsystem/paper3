#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <sstream>
#include <stdexcept>

class BST {
private:
    struct Node {
        int key;
        Node* left;
        Node* right;
        explicit Node(int k) : key(k), left(nullptr), right(nullptr) {}
    };
    Node* root;

    static bool isIntSafe(int /*value*/) {
        return true; // All int values acceptable in this implementation
    }

    static Node* minNode(Node* node) {
        Node* curr = node;
        while (curr && curr->left) curr = curr->left;
        return curr;
    }

    static Node* deleteMin(Node* node) {
        if (!node) return nullptr;
        if (!node->left) {
            Node* r = node->right;
            delete node;
            return r;
        }
        node->left = deleteMin(node->left);
        return node;
    }

    static Node* deleteRec(Node* node, int key, bool& found) {
        if (!node) return nullptr;
        if (key < node->key) {
            node->left = deleteRec(node->left, key, found);
        } else if (key > node->key) {
            node->right = deleteRec(node->right, key, found);
        } else {
            found = true;
            if (!node->left) {
                Node* r = node->right;
                delete node;
                return r;
            }
            if (!node->right) {
                Node* l = node->left;
                delete node;
                return l;
            }
            Node* succ = minNode(node->right);
            Node* newNode = new Node(succ->key);
            newNode->left = node->left;
            newNode->right = deleteMin(node->right);
            delete node;
            return newNode;
        }
        return node;
    }

    static void inorderRec(Node* node, std::vector<int>& out) {
        if (!node) return;
        inorderRec(node->left, out);
        out.push_back(node->key);
        inorderRec(node->right, out);
    }

    static void freeRec(Node* node) {
        if (!node) return;
        freeRec(node->left);
        freeRec(node->right);
        delete node;
    }

public:
    BST() : root(nullptr) {}
    ~BST() { freeRec(root); }

    bool insert(int key) {
        if (!isIntSafe(key)) return false;
        if (!root) {
            root = new Node(key);
            return true;
        }
        Node* curr = root;
        Node* parent = nullptr;
        while (curr) {
            if (key == curr->key) return false;
            parent = curr;
            curr = (key < curr->key) ? curr->left : curr->right;
        }
        if (key < parent->key) parent->left = new Node(key);
        else parent->right = new Node(key);
        return true;
    }

    bool search(int key) const {
        if (!isIntSafe(key)) return false;
        Node* curr = root;
        while (curr) {
            if (key == curr->key) return true;
            curr = (key < curr->key) ? curr->left : curr->right;
        }
        return false;
    }

    bool erase(int key) {
        if (!isIntSafe(key)) return false;
        bool found = false;
        root = deleteRec(root, key, found);
        return found;
    }

    std::vector<int> inorder() const {
        std::vector<int> out;
        inorderRec(root, out);
        return out;
    }

    static std::string vecToString(const std::vector<int>& v) {
        std::ostringstream oss;
        oss.exceptions(std::ios::failbit | std::ios::badbit);
        for (size_t i = 0; i < v.size(); ++i) {
            oss << v[i];
            if (i + 1 < v.size()) oss << " ";
        }
        return oss.str();
    }
};

int main() {
    BST bst;

    // Test Case 1: Insert elements and print inorder
    int values[] = {50, 30, 70, 20, 40, 60, 80};
    for (int v : values) {
        std::cout << "Insert " << v << ": " << (bst.insert(v) ? "true" : "false") << "\n";
    }
    std::cout << "Inorder after inserts: " << BST::vecToString(bst.inorder()) << "\n";

    // Test Case 2: Search existing and non-existing keys
    std::cout << "Search 60: " << (bst.search(60) ? "true" : "false") << "\n";
    std::cout << "Search 25: " << (bst.search(25) ? "true" : "false") << "\n";

    // Test Case 3: Delete a leaf node
    std::cout << "Delete 20 (leaf): " << (bst.erase(20) ? "true" : "false") << "\n";
    std::cout << "Inorder after deleting 20: " << BST::vecToString(bst.inorder()) << "\n";

    // Test Case 4: Delete a node with one child
    std::cout << "Delete 30 (one child): " << (bst.erase(30) ? "true" : "false") << "\n";
    std::cout << "Inorder after deleting 30: " << BST::vecToString(bst.inorder()) << "\n";

    // Test Case 5: Delete a node with two children and try duplicate insert
    std::cout << "Delete 50 (two children): " << (bst.erase(50) ? "true" : "false") << "\n";
    std::cout << "Inorder after deleting 50: " << BST::vecToString(bst.inorder()) << "\n";
    std::cout << "Insert duplicate 70: " << (bst.insert(70) ? "true" : "false") << "\n";

    return 0;
}