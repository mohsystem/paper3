#include <iostream>
#include <vector>
#include <stack>

class Task151 {
public:
    struct Node {
        int key;
        Node* left;
        Node* right;
        explicit Node(int k) : key(k), left(nullptr), right(nullptr) {}
    };

    class BST {
    public:
        BST() : root(nullptr) {}
        ~BST() { clear(); }

        bool insert(int key) {
            if (!root) {
                root = new Node(key);
                return true;
            }
            Node* parent = nullptr;
            Node* curr = root;
            while (curr) {
                parent = curr;
                if (key == curr->key) return false;
                if (key < curr->key) curr = curr->left;
                else curr = curr->right;
            }
            if (key < parent->key) parent->left = new Node(key);
            else parent->right = new Node(key);
            return true;
        }

        bool search(int key) const {
            Node* curr = root;
            while (curr) {
                if (key == curr->key) return true;
                curr = (key < curr->key) ? curr->left : curr->right;
            }
            return false;
        }

        bool deleteKey(int key) {
            Node* parent = nullptr;
            Node* curr = root;
            while (curr && curr->key != key) {
                parent = curr;
                curr = (key < curr->key) ? curr->left : curr->right;
            }
            if (!curr) return false;

            if (curr->left && curr->right) {
                Node* succParent = curr;
                Node* succ = curr->right;
                while (succ->left) {
                    succParent = succ;
                    succ = succ->left;
                }
                curr->key = succ->key;
                parent = succParent;
                curr = succ;
            }

            Node* child = curr->left ? curr->left : curr->right;
            if (!parent) {
                root = child;
            } else if (parent->left == curr) {
                parent->left = child;
            } else {
                parent->right = child;
            }
            delete curr;
            return true;
        }

        std::vector<int> inorder() const {
            std::vector<int> res;
            std::stack<Node*> st;
            Node* curr = root;
            while (curr || !st.empty()) {
                while (curr) {
                    st.push(curr);
                    curr = curr->left;
                }
                curr = st.top(); st.pop();
                res.push_back(curr->key);
                curr = curr->right;
            }
            return res;
        }

        void clear() {
            if (!root) return;
            std::stack<Node*> st;
            st.push(root);
            while (!st.empty()) {
                Node* n = st.top(); st.pop();
                if (n->left) st.push(n->left);
                if (n->right) st.push(n->right);
                delete n;
            }
            root = nullptr;
        }

    private:
        Node* root;
    };

    static void printVec(const std::vector<int>& v) {
        std::cout << "[";
        for (size_t i = 0; i < v.size(); ++i) {
            if (i) std::cout << ", ";
            std::cout << v[i];
        }
        std::cout << "]";
    }

    static int mainCpp() {
        BST tree;
        int initial[] = {5, 3, 7, 2, 4, 6, 8};
        for (int x : initial) tree.insert(x);

        std::cout << "Search 4 (expect true): " << (tree.search(4) ? "true" : "false") << "\n";
        std::cout << "Search 10 (expect false): " << (tree.search(10) ? "true" : "false") << "\n";

        tree.deleteKey(2);
        std::cout << "After deleting 2 (leaf): ";
        printVec(tree.inorder());
        std::cout << "\n";

        tree.deleteKey(3);
        std::cout << "After deleting 3 (one child): ";
        printVec(tree.inorder());
        std::cout << "\n";

        tree.deleteKey(5);
        std::cout << "After deleting 5 (two children): ";
        printVec(tree.inorder());
        std::cout << "\n";

        return 0;
    }
};

int main() {
    return Task151::mainCpp();
}