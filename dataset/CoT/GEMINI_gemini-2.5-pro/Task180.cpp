#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <set>

using namespace std;

// Type alias for our adjacency list representation
using Graph = map<int, vector<int>>;

// Forward declaration of the DFS helper function
void buildNewTree(int currentNode, int parentNode, const Graph& originalAdj, Graph& newAdj);

/**
 * Reparents a tree on a selected node.
 *
 * @param originalAdj The original undirected tree represented as an adjacency list.
 * @param newRoot     The node to become the new root.
 * @return A new directed tree (adjacency list) rooted at newRoot.
 */
Graph reparentTree(const Graph& originalAdj, int newRoot) {
    Graph newAdj;
    if (originalAdj.find(newRoot) == originalAdj.end()) {
        return newAdj; // Or throw an exception
    }

    for (const auto& pair : originalAdj) {
        newAdj[pair.first] = {};
    }

    buildNewTree(newRoot, -1, originalAdj, newAdj);
    
    // Sort children for consistent output
    for (auto& pair : newAdj) {
        sort(pair.second.begin(), pair.second.end());
    }

    return newAdj;
}

/**
 * DFS helper to build the new tree structure.
 */
void buildNewTree(int currentNode, int parentNode, const Graph& originalAdj, Graph& newAdj) {
    // .at() is used to get a const reference and will throw if key doesn't exist.
    // We've already ensured all keys exist in the calling function.
    for (int neighbor : originalAdj.at(currentNode)) {
        if (neighbor != parentNode) {
            newAdj[currentNode].push_back(neighbor);
            buildNewTree(neighbor, currentNode, originalAdj, newAdj);
        }
    }
}

// Helper to build an undirected graph from edges
Graph buildGraphFromEdges(const vector<pair<int, int>>& edges) {
    Graph adj;
    set<int> nodes;
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
    return adj;
}

// Helper to print a graph
void printGraph(const Graph& adj) {
    cout << "{";
    for (auto it = adj.begin(); it != adj.end(); ++it) {
        cout << it->first << "=[";
        for (size_t i = 0; i < it->second.size(); ++i) {
            cout << it->second[i] << (i == it->second.size() - 1 ? "" : ", ");
        }
        cout << "]";
        if (next(it) != adj.end()) {
            cout << ", ";
        }
    }
    cout << "}" << endl;
}

int main() {
    // Test Case 1: Example from prompt
    vector<pair<int, int>> edges1 = {{0, 1}, {0, 2}, {0, 3}, {1, 4}, {1, 5}, {2, 6}, {2, 7}, {3, 8}, {3, 9}};
    int newRoot1 = 6;
    Graph graph1 = buildGraphFromEdges(edges1);
    cout << "Test Case 1: Reparent on " << newRoot1 << endl;
    cout << "Original Tree: "; printGraph(graph1);
    cout << "Reparented Tree: "; printGraph(reparentTree(graph1, newRoot1));
    cout << endl;

    // Test Case 2: Line graph
    vector<pair<int, int>> edges2 = {{0, 1}, {1, 2}, {2, 3}, {3, 4}};
    int newRoot2 = 2;
    Graph graph2 = buildGraphFromEdges(edges2);
    cout << "Test Case 2: Reparent on " << newRoot2 << endl;
    cout << "Original Tree: "; printGraph(graph2);
    cout << "Reparented Tree: "; printGraph(reparentTree(graph2, newRoot2));
    cout << endl;

    // Test Case 3: Root is already the 'natural' root
    int newRoot3 = 0;
    cout << "Test Case 3: Reparent on " << newRoot3 << endl;
    cout << "Original Tree: "; printGraph(graph1);
    cout << "Reparented Tree: "; printGraph(reparentTree(graph1, newRoot3));
    cout << endl;

    // Test Case 4: Leaf node as new root
    int newRoot4 = 9;
    cout << "Test Case 4: Reparent on " << newRoot4 << endl;
    cout << "Original Tree: "; printGraph(graph1);
    cout << "Reparented Tree: "; printGraph(reparentTree(graph1, newRoot4));
    cout << endl;
        
    // Test Case 5: Small tree
    vector<pair<int, int>> edges5 = {{1, 0}, {0, 2}};
    int newRoot5 = 1;
    Graph graph5 = buildGraphFromEdges(edges5);
    cout << "Test Case 5: Reparent on " << newRoot5 << endl;
    cout << "Original Tree: "; printGraph(graph5);
    cout << "Reparented Tree: "; printGraph(reparentTree(graph5, newRoot5));
    cout << endl;

    return 0;
}