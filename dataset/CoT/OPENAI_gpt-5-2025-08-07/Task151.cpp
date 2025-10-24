#include <bits/stdc++.h>
using namespace std;

// Chain-of-Through Steps: Implement BST with insert, delete, search, secure memory handling

class BST {
private:
    struct Node {
        int key;
        Node* left;
        Node* right;
        explicit Node(int k) : key(k), left(nullptr), right(nullptr) {}
    };
    Node* root;

    Node* insertRec(Node* node, int key, bool& inserted) {
        if (!node) { inserted = true; return new (nothrow) Node(key); }
        if (key < node->key) node->left = insertRec(node->left, key, inserted);
        else if (key > node->key) node->right = insertRec(node->right, key, inserted);
        else inserted = false; // duplicate
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
            // two children: inorder successor
            Node* succParent = node;
            Node* succ = node->right;
            while (succ->left) { succParent = succ; succ = succ->left; }
            node->key = succ->key;
            if (succParent->left == succ) succParent->left = deleteRec(succParent->left, succ->key, deleted = false);
            else succParent->right = deleteRec(succParent->right, succ->key, deleted = false);
        }
        return node;
    }

    void inorderRec(Node* node, vector<int>& out) const {
        if (!node) return;
        inorderRec(node->left, out);
        out.push_back(node->key);
        inorderRec(node->right, out);
    }

    void destroy(Node* node) {
        if (!node) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

public:
    BST() : root(nullptr) {}
    ~BST() { destroy(root); }

    bool insert(int key) {
        bool inserted = false;
        root = insertRec(root, key, inserted);
        return inserted;
    }

    bool search(int key) const {
        Node* cur = root;
        while (cur) {
            if (key == cur->key) return true;
            cur = (key < cur->key) ? cur->left : cur->right;
        }
        return false;
    }

    bool erase(int key) {
        bool deleted = false;
        root = deleteRec(root, key, deleted);
        return deleted;
    }

    vector<int> inorder() const {
        vector<int> res;
        res.reserve(64);
        inorderRec(root, res);
        return res;
    }
};

static void printVec(const string& prefix, const vector<int>& v) {
    cout << prefix << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) cout << ", ";
        cout << v[i];
    }
    cout << "]\n";
}

int main() {
    // Test case 1: Insert and inorder
    BST bst;
    int vals[] = {50, 30, 20, 40, 70, 60, 80};
    for (int v : vals) bst.insert(v);
    printVec("Inorder after inserts: ", bst.inorder());

    // Test case 2: Search existing
    cout << "Search 40: " << (bst.search(40) ? "true" : "false") << "\n";

    // Test case 3: Search non-existing
    cout << "Search 25: " << (bst.search(25) ? "true" : "false") << "\n";

    // Test case 4: Delete leaf (20)
    cout << "Delete 20: " << (bst.erase(20) ? "true" : "false") << "\n";
    printVec("Inorder after deleting 20: ", bst.inorder());

    // Test case 5: Delete node with one child (30) and two children (50)
    cout << "Delete 30: " << (bst.erase(30) ? "true" : "false") << "\n";
    cout << "Delete 50: " << (bst.erase(50) ? "true" : "false") << "\n";
    printVec("Inorder after deleting 30 and 50: ", bst.inorder());

    return 0;
}