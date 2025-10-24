#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <set>
#include <string>

struct Node {
    int id;
    std::vector<Node*> children;

    Node(int i) : id(i) {}

    ~Node() {
        for (Node* child : children) {
            delete child;
        }
    }
};

Node* buildTreeRecursive(int currentNodeId, int parentId, std::map<int, std::vector<int>>& adj) {
    Node* node = new Node(currentNodeId);
    if (adj.count(currentNodeId)) {
        for (int neighborId : adj.at(currentNodeId)) {
            if (neighborId != parentId) {
                node->children.push_back(buildTreeRecursive(neighborId, currentNodeId, adj));
            }
        }
    }
    return node;
}

Node* reparentTree(const std::vector<std::pair<int, int>>& edges, int newRoot) {
    if (edges.empty()) {
        return new Node(newRoot);
    }
    
    std::map<int, std::vector<int>> adj;
    std::set<int> nodes;

    for (const auto& edge : edges) {
        adj[edge.first].push_back(edge.second);
        adj[edge.second].push_back(edge.first);
        nodes.insert(edge.first);
        nodes.insert(edge.second);
    }
    
    if (nodes.find(newRoot) == nodes.end()) {
        return nullptr;
    }

    for (auto& pair : adj) {
        std::sort(pair.second.begin(), pair.second.end());
    }

    return buildTreeRecursive(newRoot, -1, adj);
}

void printTree(const Node* node, const std::string& prefix = "") {
    if (node == nullptr) {
        std::cout << "Tree is null." << std::endl;
        return;
    }
    std::cout << prefix << node->id << std::endl;
    for (const Node* child : node->children) {
        printTree(child, prefix + "  ");
    }
}

void runTestCase(int testNum, const std::vector<std::pair<int, int>>& edges, int newRoot) {
    std::cout << "--- Test Case " << testNum << " ---" << std::endl;
    std::cout << "Reparenting on node " << newRoot << std::endl;
    Node* newTree = reparentTree(edges, newRoot);
    printTree(newTree);
    delete newTree;
    std::cout << std::endl;
}

int main() {
    // Test Case 1: From prompt
    std::vector<std::pair<int, int>> edges1 = {{0, 1}, {0, 2}, {0, 3}, {1, 4}, {1, 5}, {2, 6}, {2, 7}, {3, 8}, {3, 9}};
    runTestCase(1, edges1, 6);

    // Test Case 2: Reparent on original root
    runTestCase(2, edges1, 0);

    // Test Case 3: Reparent on a leaf
    runTestCase(3, edges1, 9);

    // Test Case 4: Smaller tree
    std::vector<std::pair<int, int>> edges4 = {{1, 0}, {2, 0}, {3, 1}, {4, 1}};
    runTestCase(4, edges4, 1);

    // Test Case 5: Line graph
    std::vector<std::pair<int, int>> edges5 = {{0, 1}, {1, 2}, {2, 3}};
    runTestCase(5, edges5, 2);
    
    return 0;
}