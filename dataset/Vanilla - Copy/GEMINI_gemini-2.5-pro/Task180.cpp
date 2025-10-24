#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <algorithm>

struct Node {
    int value;
    std::vector<Node*> children;

    Node(int val) : value(val) {}

    Node* addChild(Node* child) {
        children.push_back(child);
        return this;
    }
};

void buildAdjacencyList(Node* node, std::unordered_map<int, std::vector<int>>& adj, std::unordered_set<int>& nodes) {
    if (!node) {
        return;
    }
    nodes.insert(node->value);
    adj.try_emplace(node->value);
    for (Node* child : node->children) {
        adj.try_emplace(child->value);
        adj[node->value].push_back(child->value);
        adj[child->value].push_back(node->value);
        buildAdjacencyList(child, adj, nodes);
    }
}

Node* buildNewTreeFromAdjacencyList(int rootValue, std::unordered_map<int, std::vector<int>>& adj) {
    Node* newRoot = new Node(rootValue);
    std::queue<Node*> q;
    q.push(newRoot);
    std::unordered_set<int> visited;
    visited.insert(rootValue);

    while (!q.empty()) {
        Node* currentNode = q.front();
        q.pop();

        if (adj.count(currentNode->value)) {
            // Sort neighbors for deterministic output
            std::sort(adj.at(currentNode->value).begin(), adj.at(currentNode->value).end());
            for (int neighborValue : adj.at(currentNode->value)) {
                if (visited.find(neighborValue) == visited.end()) {
                    visited.insert(neighborValue);
                    Node* childNode = new Node(neighborValue);
                    currentNode->children.push_back(childNode);
                    q.push(childNode);
                }
            }
        }
    }
    return newRoot;
}

Node* reparentTree(Node* oldRoot, int newRootValue) {
    if (!oldRoot) {
        return nullptr;
    }

    std::unordered_map<int, std::vector<int>> adj;
    std::unordered_set<int> nodes;
    buildAdjacencyList(oldRoot, adj, nodes);

    if (nodes.find(newRootValue) == nodes.end()) {
        return nullptr; 
    }

    return buildNewTreeFromAdjacencyList(newRootValue, adj);
}

void deleteTree(Node* node) {
    if (!node) {
        return;
    }
    for (Node* child : node->children) {
        deleteTree(child);
    }
    delete node;
}

void printTree(Node* node, const std::string& prefix = "", bool isTail = true) {
    if (!node) return;
    std::cout << prefix << (isTail ? "└── " : "├── ") << node->value << std::endl;
    for (size_t i = 0; i < node->children.size(); ++i) {
        bool isLastChild = (i == node->children.size() - 1);
        printTree(node->children[i], prefix + (isTail ? "    " : "│   "), isLastChild);
    }
}

void runTestCase(const std::string& name, Node* root, int newRootValue) {
    std::cout << "--- " << name << " ---" << std::endl;
    std::cout << "Original tree (rooted at " << root->value << "):" << std::endl;
    printTree(root);

    Node* newRoot = reparentTree(root, newRootValue);
    std::cout << "\nNew tree (re-rooted at " << newRootValue << "):" << std::endl;
    printTree(newRoot);

    deleteTree(newRoot);
    std::cout << "\n" << std::string(40, '=') << std::endl;
}

int main() {
    // Test Case 1: Example from prompt
    Node* root1 = new Node(0);
    Node* n1 = new Node(1);
    Node* n2 = new Node(2);
    Node* n3 = new Node(3);
    n1->addChild(new Node(4))->addChild(new Node(5));
    n2->addChild(new Node(6))->addChild(new Node(7));
    n3->addChild(new Node(8))->addChild(new Node(9));
    root1->addChild(n1)->addChild(n2)->addChild(n3);
    runTestCase("Test Case 1: Re-parenting on node 6", root1, 6);

    // Test Case 2: Re-rooting at the current root (0)
    runTestCase("Test Case 2: Re-parenting on current root 0", root1, 0);

    // Test Case 3: Re-rooting at a leaf node (9)
    runTestCase("Test Case 3: Re-parenting on leaf node 9", root1, 9);
    
    deleteTree(root1);

    // Test Case 4: Linear tree
    Node* root4 = new Node(0);
    root4->addChild(new Node(1)->addChild(new Node(2)->addChild(new Node(3))));
    runTestCase("Test Case 4: Linear tree", root4, 2);
    deleteTree(root4);

    // Test Case 5: Tree with a single node
    Node* root5 = new Node(42);
    runTestCase("Test Case 5: Single node tree", root5, 42);
    deleteTree(root5);

    return 0;
}