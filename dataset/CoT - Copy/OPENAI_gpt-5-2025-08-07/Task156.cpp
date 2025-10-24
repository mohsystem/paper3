#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <stack>
#include <cctype>

/*
 Chain-of-Through process:
 1) Problem: DFS traversals on a binary tree given traversal type.
 2) Security: Validate order, avoid recursion depth via iterative methods, handle empty inputs.
 3) Secure coding: No UB, bounds-checked loops, safe stoi usage in controlled test data.
 4) Review: Ensure null handling and stack usage correct.
 5) Output: Final secure iterative implementations.
*/

struct Node {
    int val;
    Node* left;
    Node* right;
    explicit Node(int v): val(v), left(nullptr), right(nullptr) {}
};

static std::string normalizeOrder(const std::string& order) {
    std::string o;
    o.reserve(order.size());
    for (char c : order) o.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    if (o == "preorder" || o == "inorder" || o == "postorder") return o;
    return "preorder";
}

std::vector<int> dfs(Node* root, const std::string& order) {
    std::string ord = normalizeOrder(order);
    std::vector<int> res;
    if (!root) return res;

    if (ord == "preorder") {
        std::stack<Node*> st;
        st.push(root);
        while (!st.empty()) {
            Node* n = st.top(); st.pop();
            res.push_back(n->val);
            if (n->right) st.push(n->right);
            if (n->left) st.push(n->left);
        }
    } else if (ord == "inorder") {
        std::stack<Node*> st;
        Node* cur = root;
        while (cur || !st.empty()) {
            while (cur) {
                st.push(cur);
                cur = cur->left;
            }
            Node* n = st.top(); st.pop();
            res.push_back(n->val);
            cur = n->right;
        }
    } else { // postorder
        std::stack<Node*> st;
        Node* cur = root;
        Node* last = nullptr;
        while (cur || !st.empty()) {
            if (cur) {
                st.push(cur);
                cur = cur->left;
            } else {
                Node* peek = st.top();
                if (peek->right && last != peek->right) {
                    cur = peek->right;
                } else {
                    res.push_back(peek->val);
                    last = peek;
                    st.pop();
                }
            }
        }
    }
    return res;
}

// Build tree from level-order vector of strings; "null" indicates missing
Node* buildTreeFromLevelOrder(const std::vector<std::string>& vals) {
    if (vals.empty() || vals[0] == "null") return nullptr;
    Node* root = new Node(std::stoi(vals[0]));
    std::queue<Node*> q;
    q.push(root);
    size_t i = 1;
    while (!q.empty() && i < vals.size()) {
        Node* cur = q.front(); q.pop();
        if (i < vals.size() && vals[i] != "null") {
            cur->left = new Node(std::stoi(vals[i]));
            q.push(cur->left);
        }
        ++i;
        if (i < vals.size() && vals[i] != "null") {
            cur->right = new Node(std::stoi(vals[i]));
            q.push(cur->right);
        }
        ++i;
    }
    return root;
}

void freeTree(Node* root) {
    if (!root) return;
    std::stack<Node*> st;
    Node* cur = root;
    Node* last = nullptr;
    while (cur || !st.empty()) {
        if (cur) {
            st.push(cur);
            cur = cur->left;
        } else {
            Node* peek = st.top();
            if (peek->right && last != peek->right) {
                cur = peek->right;
            } else {
                last = peek;
                st.pop();
                delete peek;
            }
        }
    }
}

static void printVec(const std::vector<int>& v) {
    std::cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << v[i];
    }
    std::cout << "]";
}

int main() {
    std::vector<std::vector<std::string>> tests = {
        {"1","2","3","4","5","null","6"},
        {"1","null","2","3"},
        {},
        {"10","5","15","null","7","12","20"},
        {"42"}
    };
    std::vector<std::string> orders = {"preorder","inorder","postorder"};
    for (size_t i = 0; i < tests.size(); ++i) {
        Node* root = buildTreeFromLevelOrder(tests[i]);
        std::cout << "Case " << (i+1) << ":\n";
        for (const auto& o : orders) {
            auto res = dfs(root, o);
            std::cout << " " << o << ": ";
            printVec(res);
            std::cout << "\n";
        }
        freeTree(root);
    }
    return 0;
}