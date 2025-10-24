#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <set>
#include <utility>

// Using std::map to allow for non-contiguous node IDs and preserve insertion order.
using AdjacencyList = std::map<int, std::vector<int>>;

void buildNewTreeDfs(int currentNode, int parentNode,
                     const AdjacencyList& oldAdj,
                     AdjacencyList& newTree) {
    
    // Ensure the current node has an entry in the new tree.
    // The key insertion order in std::map creates a pre-order traversal view.
    newTree[currentNode] = {};

    auto it = oldAdj.find(currentNode);
    if (it == oldAdj.end()) {
        return;
    }

    std::vector<int> neighbors = it->second;
    std::sort(neighbors.begin(), neighbors.end()); // Sort for deterministic output

    for (int neighbor : neighbors) {
        if (neighbor != parentNode) {
            newTree[currentNode].push_back(neighbor);
            buildNewTreeDfs(neighbor, currentNode, oldAdj, newTree);
        }
    }
}

AdjacencyList reparentTree(const AdjacencyList& adjacencyList, int newRoot) {
    AdjacencyList newTree;
    buildNewTreeDfs(newRoot, -1, adjacencyList, newTree);
    return newTree;
}

// --- Test Cases ---
void buildAdjList(AdjacencyList& adj, const std::vector<std::pair<int, int>>& edges) {
    std::set<int> nodes;
    for (const auto& edge : edges) {
        nodes.insert(edge.first);
        nodes.insert(edge.second);
    }
    for (int node : nodes) {
        adj[node] = {};
    }
    for (const auto& edge : edges) {
        adj[edge.first].push_back(edge.second);
        adj[edge.second].push_back(edge.first);
    }
}

void printMap(const AdjacencyList& map) {
    for (const auto& pair : map) {
        std::cout << pair.first << " -> [";
        for (size_t i = 0; i < pair.second.size(); ++i) {
            std::cout << pair.second[i] << (i == pair.second.size() - 1 ? "" : ", ");
        }
        std::cout << "]" << std::endl;
    }
}

void runTestCase(int testNum, const std::string& description, const std::vector<std::pair<int, int>>& edges, int newRoot) {
    std::cout << "--- Test Case " << testNum << ": " << description << " ---" << std::endl;
    
    AdjacencyList adjacencyList;
    buildAdjList(adjacencyList, edges);
    
    std::cout << "Reparenting on node: " << newRoot << std::endl;

    AdjacencyList reparentedTree = reparentTree(adjacencyList, newRoot);
    std::cout << "Reparented Tree (Parent -> Children):" << std::endl;
    printMap(reparentedTree);
    std::cout << std::endl;
}

int main() {
    runTestCase(1, "Example from prompt",
        {{0, 1}, {0, 2}, {0, 3}, {1, 4}, {1, 5}, {2, 6}, {2, 7}, {3, 8}, {3, 9}}, 6);
    runTestCase(2, "Simple line graph",
        {{0, 1}, {1, 2}, {2, 3}, {3, 4}}, 2);
    runTestCase(3, "Star graph, reparent on root",
        {{0, 1}, {0, 2}, {0, 3}, {0, 4}}, 0);
    runTestCase(4, "Star graph, reparent on a leaf",
        {{0, 1}, {0, 2}, {0, 3}, {0, 4}}, 3);
    runTestCase(5, "A more complex tree",
        {{1, 2}, {1, 3}, {1, 4}, {2, 5}, {2, 6}, {4, 7}, {4, 8}}, 4);
    return 0;
}