#include <bits/stdc++.h>
using namespace std;

struct TreeNode {
    int val;
    TreeNode* left;
    TreeNode* right;
    explicit TreeNode(int v): val(v), left(nullptr), right(nullptr) {}
};

static inline string to_lower_trim(const string& s) {
    string r;
    r.reserve(s.size());
    for (char c : s) {
        if (!isspace(static_cast<unsigned char>(c))) r.push_back(static_cast<char>(tolower(static_cast<unsigned char>(c))));
        else r.push_back(c);
    }
    // trim
    size_t start = r.find_first_not_of(' ');
    if (start == string::npos) return "";
    size_t end = r.find_last_not_of(' ');
    string t = r.substr(start, end - start + 1);
    for (char& c : t) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    return t;
}

vector<int> dfs(TreeNode* root, const string& order) {
    string ord = to_lower_trim(order);
    if (ord == "preorder") {
        vector<int> out;
        if (!root) return out;
        vector<TreeNode*> st;
        st.push_back(root);
        while (!st.empty()) {
            TreeNode* n = st.back(); st.pop_back();
            out.push_back(n->val);
            if (n->right) st.push_back(n->right);
            if (n->left) st.push_back(n->left);
        }
        return out;
    } else if (ord == "inorder") {
        vector<int> out;
        vector<TreeNode*> st;
        TreeNode* curr = root;
        while (curr || !st.empty()) {
            while (curr) { st.push_back(curr); curr = curr->left; }
            curr = st.back(); st.pop_back();
            out.push_back(curr->val);
            curr = curr->right;
        }
        return out;
    } else if (ord == "postorder") {
        vector<int> out;
        if (!root) return out;
        vector<TreeNode*> st;
        TreeNode* curr = root;
        TreeNode* last = nullptr;
        while (curr || !st.empty()) {
            if (curr) {
                st.push_back(curr);
                curr = curr->left;
            } else {
                TreeNode* peek = st.back();
                if (peek->right && last != peek->right) {
                    curr = peek->right;
                } else {
                    out.push_back(peek->val);
                    st.pop_back();
                    last = peek;
                }
            }
        }
        return out;
    } else {
        throw invalid_argument("Unsupported order");
    }
}

static void print_vec(const string& label, const vector<int>& v) {
    cout << label << ": [";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) cout << ", ";
        cout << v[i];
    }
    cout << "]\n";
}

int main() {
    // Test 1: Empty tree
    TreeNode* t1 = nullptr;
    print_vec("Test1 - Preorder (empty)", dfs(t1, "preorder"));

    // Test 2: Single node
    TreeNode* t2 = new TreeNode(42);
    print_vec("Test2 - Inorder (single)", dfs(t2, "inorder"));

    // Test 3: Balanced tree
    TreeNode* t3 = new TreeNode(4);
    t3->left = new TreeNode(2);
    t3->right = new TreeNode(6);
    t3->left->left = new TreeNode(1);
    t3->left->right = new TreeNode(3);
    t3->right->left = new TreeNode(5);
    t3->right->right = new TreeNode(7);
    print_vec("Test3 - Inorder (balanced)", dfs(t3, "inorder"));

    // Test 4: Left-skewed
    TreeNode* t4 = new TreeNode(5);
    t4->left = new TreeNode(4);
    t4->left->left = new TreeNode(3);
    t4->left->left->left = new TreeNode(2);
    t4->left->left->left->left = new TreeNode(1);
    print_vec("Test4 - Postorder (left-skewed)", dfs(t4, "postorder"));

    // Test 5: Mixed tree
    TreeNode* t5 = new TreeNode(8);
    t5->left = new TreeNode(3);
    t5->right = new TreeNode(10);
    t5->left->left = new TreeNode(1);
    t5->left->right = new TreeNode(6);
    t5->left->right->left = new TreeNode(4);
    t5->left->right->right = new TreeNode(7);
    t5->right->right = new TreeNode(14);
    t5->right->right->left = new TreeNode(13);
    print_vec("Test5 - Preorder (mixed)", dfs(t5, "preorder"));

    // Cleanup (optional)
    // For brevity, not deleting all nodes. In production, ensure to free all allocated nodes.

    return 0;
}