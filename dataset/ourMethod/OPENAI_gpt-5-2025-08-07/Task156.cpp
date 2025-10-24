#include <bits/stdc++.h>
using namespace std;

struct TreeNode {
    int val;
    TreeNode* left;
    TreeNode* right;
    explicit TreeNode(int v) : val(v), left(nullptr), right(nullptr) {}
};

TreeNode* buildTree(const vector<int>& values, const vector<int>& present) {
    if (values.size() != present.size()) {
        throw invalid_argument("values and present must have same length");
    }
    size_t n = values.size();
    if (n == 0) return nullptr;
    if (present[0] == 0) return nullptr;

    vector<TreeNode*> nodes(n, nullptr);
    for (size_t i = 0; i < n; ++i) {
        if (present[i]) nodes[i] = new TreeNode(values[i]);
    }
    for (size_t i = 0; i < n; ++i) {
        if (!nodes[i]) continue;
        size_t li = 2 * i + 1;
        size_t ri = 2 * i + 2;
        if (li < n && nodes[li]) nodes[i]->left = nodes[li];
        if (ri < n && nodes[ri]) nodes[i]->right = nodes[ri];
    }
    return nodes[0];
}

void preorder(TreeNode* node, vector<int>& out) {
    if (!node) return;
    out.push_back(node->val);
    preorder(node->left, out);
    preorder(node->right, out);
}

void inorder(TreeNode* node, vector<int>& out) {
    if (!node) return;
    inorder(node->left, out);
    out.push_back(node->val);
    inorder(node->right, out);
}

void postorder(TreeNode* node, vector<int>& out) {
    if (!node) return;
    postorder(node->left, out);
    postorder(node->right, out);
    out.push_back(node->val);
}

vector<int> traverse(TreeNode* root, const string& order) {
    string o = order;
    for (char& c : o) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    vector<int> res;
    if (o == "preorder") {
        preorder(root, res);
    } else if (o == "inorder") {
        inorder(root, res);
    } else if (o == "postorder") {
        postorder(root, res);
    } else {
        throw invalid_argument("Unsupported order: " + order);
    }
    return res;
}

string formatList(const vector<int>& v) {
    string s = "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) s += ",";
        s += to_string(v[i]);
    }
    s += "]";
    return s;
}

void freeTree(TreeNode* node) {
    if (!node) return;
    freeTree(node->left);
    freeTree(node->right);
    delete node;
}

int main() {
    vector<vector<int>> testsValues = {
        {1,2,3,4,5,6,7},
        {1,2,3,0,5,0,7},
        {42},
        {},
        {1,0,2,0,0,0,3}
    };
    vector<vector<int>> testsPresent = {
        {1,1,1,1,1,1,1},
        {1,1,1,0,1,0,1},
        {1},
        {},
        {1,0,1,0,0,0,1}
    };
    vector<string> orders = {"preorder","inorder","postorder"};

    for (size_t t = 0; t < testsValues.size(); ++t) {
        cout << "Test #" << (t + 1) << "\n";
        try {
            TreeNode* root = buildTree(testsValues[t], testsPresent[t]);
            for (const auto& ord : orders) {
                vector<int> res = traverse(root, ord);
                cout << ord << ": " << formatList(res) << "\n";
            }
            freeTree(root);
        } catch (const exception& ex) {
            cout << "Error: " << ex.what() << "\n";
        }
        cout << "\n";
    }
    return 0;
}