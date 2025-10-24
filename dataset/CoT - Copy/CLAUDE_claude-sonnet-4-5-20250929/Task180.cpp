
#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <string>

using namespace std;

class TreeNode {
public:
    int value;
    vector<TreeNode*> children;
    TreeNode* parent;
    
    TreeNode(int val) : value(val), parent(nullptr) {}
    
    ~TreeNode() {
        for (TreeNode* child : children) {
            delete child;
        }
    }
};

class Task180 {
public:
    static TreeNode* findNode(TreeNode* root, int value) {
        if (root == nullptr) {
            return nullptr;
        }
        if (root->value == value) {
            return root;
        }
        for (TreeNode* child : root->children) {
            TreeNode* found = findNode(child, value);
            if (found != nullptr) {
                return found;
            }
        }
        return nullptr;
    }
    
    static TreeNode* reparentTree(TreeNode* root, int targetValue) {
        if (root == nullptr) {
            return nullptr;
        }
        
        TreeNode* targetNode = findNode(root, targetValue);
        if (targetNode == nullptr) {
            return root;
        }
        
        if (targetNode == root) {
            return root;
        }
        
        vector<TreeNode*> pathToRoot;
        TreeNode* current = targetNode;
        while (current != nullptr) {
            pathToRoot.push_back(current);
            current = current->parent;
        }
        
        for (size_t i = 0; i < pathToRoot.size() - 1; i++) {
            TreeNode* child = pathToRoot[i];
            TreeNode* parent = pathToRoot[i + 1];
            
            auto it = find(parent->children.begin(), parent->children.end(), child);
            if (it != parent->children.end()) {
                parent->children.erase(it);
            }
            child->children.push_back(parent);
            parent->parent = child;
        }
        
        targetNode->parent = nullptr;
        return targetNode;
    }
    
    static TreeNode* buildTree(const vector<vector<int>>& edges) {
        if (edges.empty()) {
            return nullptr;
        }
        
        unordered_map<int, TreeNode*> nodes;
        unordered_set<int> childrenSet;
        
        for (const auto& edge : edges) {
            if (edge.size() != 2) continue;
            
            if (nodes.find(edge[0]) == nodes.end()) {
                nodes[edge[0]] = new TreeNode(edge[0]);
            }
            if (nodes.find(edge[1]) == nodes.end()) {
                nodes[edge[1]] = new TreeNode(edge[1]);
            }
            
            TreeNode* parent = nodes[edge[0]];
            TreeNode* child = nodes[edge[1]];
            
            parent->children.push_back(child);
            child->parent = parent;
            childrenSet.insert(edge[1]);
        }
        
        for (const auto& pair : nodes) {
            if (childrenSet.find(pair.first) == childrenSet.end()) {
                return pair.second;
            }
        }
        
        return nodes.empty() ? nullptr : nodes.begin()->second;
    }
    
    static void printTree(TreeNode* root, string prefix = "", bool isTail = true) {
        if (root == nullptr) return;
        cout << prefix << (isTail ? "└── " : "├── ") << root->value << endl;
        for (size_t i = 0; i < root->children.size(); i++) {
            printTree(root->children[i], 
                     prefix + (isTail ? "    " : "│   "), 
                     i == root->children.size() - 1);
        }
    }
    
    static void deleteTree(TreeNode* root) {
        if (root == nullptr) return;
        vector<TreeNode*> toDelete = root->children;
        root->children.clear();
        for (TreeNode* child : toDelete) {
            deleteTree(child);
        }
        delete root;
    }
};

int main() {
    // Test case 1
    vector<vector<int>> edges1 = {{0, 1}, {0, 2}, {0, 3}, {1, 4}, {1, 5}, {2, 6}, {2, 7}, {3, 8}, {3, 9}};
    TreeNode* root1 = Task180::buildTree(edges1);
    cout << "Test 1 - Original tree:" << endl;
    Task180::printTree(root1);
    TreeNode* newRoot1 = Task180::reparentTree(root1, 6);
    cout << "\\nTest 1 - Reparented on node 6:" << endl;
    Task180::printTree(newRoot1);
    Task180::deleteTree(newRoot1);
    
    // Test case 2
    vector<vector<int>> edges2 = {{0, 1}, {0, 2}};
    TreeNode* root2 = Task180::buildTree(edges2);
    cout << "\\n\\nTest 2 - Reparent on root:" << endl;
    TreeNode* newRoot2 = Task180::reparentTree(root2, 0);
    Task180::printTree(newRoot2);
    Task180::deleteTree(newRoot2);
    
    // Test case 3
    TreeNode* root3 = new TreeNode(0);
    cout << "\\n\\nTest 3 - Single node:" << endl;
    TreeNode* newRoot3 = Task180::reparentTree(root3, 0);
    Task180::printTree(newRoot3);
    Task180::deleteTree(newRoot3);
    
    // Test case 4
    vector<vector<int>> edges4 = {{0, 1}, {1, 2}, {2, 3}};
    TreeNode* root4 = Task180::buildTree(edges4);
    cout << "\\n\\nTest 4 - Linear tree reparented on leaf:" << endl;
    TreeNode* newRoot4 = Task180::reparentTree(root4, 3);
    Task180::printTree(newRoot4);
    Task180::deleteTree(newRoot4);
    
    // Test case 5
    vector<vector<int>> edges5 = {{0, 1}, {0, 2}};
    TreeNode* root5 = Task180::buildTree(edges5);
    cout << "\\n\\nTest 5 - Invalid target (returns original):" << endl;
    TreeNode* newRoot5 = Task180::reparentTree(root5, 99);
    Task180::printTree(newRoot5);
    Task180::deleteTree(newRoot5);
    
    return 0;
}
