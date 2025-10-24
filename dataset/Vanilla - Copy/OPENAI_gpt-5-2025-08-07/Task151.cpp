#include <iostream>
#include <vector>
#include <algorithm>

class Task151 {
private:
    struct Node {
        int key;
        Node* left;
        Node* right;
        Node(int k) : key(k), left(nullptr), right(nullptr) {}
    };
    Node* root = nullptr;

    Node* insertRec(Node* node, int key, bool& inserted) {
        if (!node) {
            inserted = true;
            return new Node(key);
        }
        if (key < node->key) node->left = insertRec(node->left, key, inserted);
        else if (key > node->key) node->right = insertRec(node->right, key, inserted);
        else inserted = false;
        return node;
    }

    Node* deleteRec(Node* node, int key, bool& deleted) {
        if (!node) return nullptr;
        if (key < node->key) node->left = deleteRec(node->left, key, deleted);
        else if (key > node->key) node->right = deleteRec(node->right, key, deleted);
        else {
            deleted = true;
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
            Node* succ = node->right;
            while (succ->left) succ = succ->left;
            node->key = succ->key;
            node->right = deleteRec(node->right, succ->key, deleted = true);
        }
        return node;
    }

    void inorderRec(Node* node, std::vector<int>& out) const {
        if (!node) return;
        inorderRec(node->left, out);
        out.push_back(node->key);
        inorderRec(node->right, out);
    }

    void freeRec(Node* node) {
        if (!node) return;
        freeRec(node->left);
        freeRec(node->right);
        delete node;
    }

public:
    ~Task151() { freeRec(root); }

    bool search(int key) const {
        Node* cur = root;
        while (cur) {
            if (key == cur->key) return true;
            if (key < cur->key) cur = cur->left; else cur = cur->right;
        }
        return false;
    }

    bool insert(int key) {
        bool inserted = false;
        root = insertRec(root, key, inserted);
        return inserted;
    }

    bool deleteKey(int key) {
        bool deleted = false;
        root = deleteRec(root, key, deleted);
        return deleted;
    }

    std::vector<int> inorder() const {
        std::vector<int> out;
        inorderRec(root, out);
        return out;
    }
};

static void printVec(const std::vector<int>& v) {
    std::cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << v[i];
    }
    std::cout << "]\n";
}

int main() {
    Task151 bst;
    // Test 1: Insert values and print inorder
    int vals[] = {5, 3, 7, 2, 4, 6, 8};
    for (int v : vals) std::cout << (bst.insert(v) ? "1" : "0") << " ";
    std::cout << "\n";
    printVec(bst.inorder());

    // Test 2: Search existing value
    std::cout << (bst.search(4) ? "1" : "0") << "\n";

    // Test 3: Search non-existing value
    std::cout << (bst.search(10) ? "1" : "0") << "\n";

    // Test 4: Delete leaf (2) and node with one child (3), then print inorder
    std::cout << (bst.deleteKey(2) ? "1" : "0") << "\n";
    std::cout << (bst.deleteKey(3) ? "1" : "0") << "\n";
    printVec(bst.inorder());

    // Test 5: Delete node with two children (7), try duplicate insert (5), delete non-existent (42)
    std::cout << (bst.deleteKey(7) ? "1" : "0") << "\n";
    std::cout << (bst.insert(5) ? "1" : "0") << "\n";
    std::cout << (bst.deleteKey(42) ? "1" : "0") << "\n";
    printVec(bst.inorder());
    return 0;
}