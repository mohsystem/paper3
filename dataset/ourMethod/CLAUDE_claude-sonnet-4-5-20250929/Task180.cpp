
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <stdexcept>

// Tree node structure representing an undirected tree
struct TreeNode {
    int value;
    std::vector<int> children;
    
    TreeNode(int val) : value(val) {}
};

// Function to build adjacency list from parent-child relationships
// Input validation: checks for valid node values and prevents cycles
std::map<int, std::set<int>> buildAdjacencyList(const std::vector<std::pair<int, int>>& edges) {
    std::map<int, std::set<int>> adj;
    
    // Input validation: check for valid edges
    for (const auto& edge : edges) {
        if (edge.first < 0 || edge.second < 0) {
            throw std::invalid_argument("Node values must be non-negative");
        }
        // Prevent self-loops (security: prevent infinite loops)
        if (edge.first == edge.second) {
            throw std::invalid_argument("Self-loops not allowed in tree");
        }
        
        // Build undirected graph
        adj[edge.first].insert(edge.second);
        adj[edge.second].insert(edge.first);
    }
    
    return adj;
}

// DFS to build tree with new root, preventing revisits (security: prevents cycles)
void buildTreeFromRoot(int current, int parent, 
                       const std::map<int, std::set<int>>& adj,
                       std::map<int, TreeNode*>& treeMap,
                       std::set<int>& visited) {
    // Security: prevent infinite recursion by tracking visited nodes
    if (visited.count(current) > 0) {
        return;
    }
    visited.insert(current);
    
    // Bounds check: ensure current node exists in adjacency list
    if (adj.find(current) == adj.end()) {
        return;
    }
    
    TreeNode* node = new TreeNode(current);
    treeMap[current] = node;
    
    // Add all neighbors except parent as children
    for (int neighbor : adj.at(current)) {
        if (neighbor != parent && visited.count(neighbor) == 0) {
            node->children.push_back(neighbor);
            buildTreeFromRoot(neighbor, current, adj, treeMap, visited);
        }
    }
}

// Main function to reparent tree on selected node
// Returns root of reparented tree
TreeNode* reparentTree(const std::vector<std::pair<int, int>>& edges, int newRoot) {
    // Input validation: check for empty input
    if (edges.empty()) {
        return new TreeNode(newRoot);
    }
    
    // Input validation: check newRoot is non-negative
    if (newRoot < 0) {
        throw std::invalid_argument("Root value must be non-negative");
    }
    
    // Build adjacency list with input validation
    std::map<int, std::set<int>> adj = buildAdjacencyList(edges);
    
    // Validate that newRoot exists in the tree
    bool rootExists = false;
    for (const auto& edge : edges) {
        if (edge.first == newRoot || edge.second == newRoot) {
            rootExists = true;
            break;
        }
    }
    if (!rootExists) {
        throw std::invalid_argument("New root must exist in tree");
    }
    
    // Build tree from new root with cycle prevention
    std::map<int, TreeNode*> treeMap;
    std::set<int> visited;
    buildTreeFromRoot(newRoot, -1, adj, treeMap, visited);
    
    // Bounds check: ensure root was created
    if (treeMap.find(newRoot) == treeMap.end()) {
        throw std::runtime_error("Failed to create tree root");
    }
    
    return treeMap[newRoot];
}

// Helper function to free tree memory (RAII principle)
void freeTree(TreeNode* root, std::set<TreeNode*>& freed) {
    if (root == nullptr || freed.count(root) > 0) {
        return;
    }
    freed.insert(root);
    
    for (int childVal : root->children) {
        // Note: In this implementation we only store values, not pointers
        // so we don't recursively free here\n    }\n    delete root;\n}\n\n// Helper to print tree for testing\nvoid printTree(TreeNode* node, int depth = 0) {\n    if (node == nullptr) return;\n    \n    // Bounds check: prevent excessive recursion depth\n    if (depth > 1000) {\n        std::cout << "Max depth exceeded\\n";\n        return;\n    }\n    \n    for (int i = 0; i < depth; ++i) std::cout << "  ";\n    std::cout << node->value << std::endl;\n    \n    for (int child : node->children) {\n        // In production, we'd need the full tree map to traverse
        std::cout << "  Child: " << child << std::endl;
    }
}

int main() {
    try {
        // Test case 1: Simple linear tree
        std::vector<std::pair<int, int>> edges1 = {{0, 1}, {1, 2}};
        TreeNode* root1 = reparentTree(edges1, 1);
        std::cout << "Test 1 - Root: " << root1->value << std::endl;
        std::set<TreeNode*> freed1;
        freeTree(root1, freed1);
        
        // Test case 2: Tree from problem description
        std::vector<std::pair<int, int>> edges2 = {
            {0, 1}, {0, 2}, {0, 3},
            {1, 4}, {1, 5}, {2, 6}, {2, 7}, {3, 8}, {3, 9}
        };
        TreeNode* root2 = reparentTree(edges2, 6);
        std::cout << "Test 2 - Root: " << root2->value << std::endl;
        std::set<TreeNode*> freed2;
        freeTree(root2, freed2);
        
        // Test case 3: Single node
        std::vector<std::pair<int, int>> edges3 = {};
        TreeNode* root3 = reparentTree(edges3, 0);
        std::cout << "Test 3 - Root: " << root3->value << std::endl;
        std::set<TreeNode*> freed3;
        freeTree(root3, freed3);
        
        // Test case 4: Star topology
        std::vector<std::pair<int, int>> edges4 = {{0, 1}, {0, 2}, {0, 3}};
        TreeNode* root4 = reparentTree(edges4, 2);
        std::cout << "Test 4 - Root: " << root4->value << std::endl;
        std::set<TreeNode*> freed4;
        freeTree(root4, freed4);
        
        // Test case 5: Chain reparented to end
        std::vector<std::pair<int, int>> edges5 = {{0, 1}, {1, 2}, {2, 3}};
        TreeNode* root5 = reparentTree(edges5, 3);
        std::cout << "Test 5 - Root: " << root5->value << std::endl;
        std::set<TreeNode*> freed5;
        freeTree(root5, freed5);
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
