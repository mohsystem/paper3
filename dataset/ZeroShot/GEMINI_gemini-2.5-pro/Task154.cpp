#include <iostream>
#include <vector>
#include <stack>
#include <algorithm> // For std::reverse

// Using a namespace to group graph-related functions
namespace GraphUtils {
    /**
     * @brief Adds a directed edge from vertex u to vertex v.
     * @param adj The adjacency list.
     * @param u The source vertex.
     * @param v The destination vertex.
     */
    void addEdge(std::vector<std::vector<int>>& adj, int u, int v) {
        // --- Security: Basic bounds check ---
        if (u >= 0 && u < adj.size() && v >= 0 && v < adj.size()) {
            adj[u].push_back(v);
        }
    }
}

/**
 * @brief Performs an iterative Depth-First Search (DFS) on a graph.
 * @param V The number of vertices in the graph.
 * @param adj The adjacency list representation of the graph.
 * @param startNode The node to start the search from.
 * @return A vector of integers representing the DFS traversal order.
 */
std::vector<int> dfs(int V, const std::vector<std::vector<int>>& adj, int startNode) {
    // --- Security: Input Validation ---
    if (V <= 0 || startNode < 0 || startNode >= V || adj.size() != V) {
        // Return an empty vector for invalid input to prevent crashes.
        return {};
    }

    std::vector<int> result;
    result.reserve(V); // Pre-allocate memory to avoid reallocations
    std::vector<bool> visited(V, false);
    std::stack<int> stack;

    stack.push(startNode);

    while (!stack.empty()) {
        int u = stack.top();
        stack.pop();

        if (!visited[u]) {
            visited[u] = true;
            result.push_back(u);

            // Get all adjacent vertices of the popped vertex u.
            // We iterate in reverse to approximate the order of a recursive DFS.
            const auto& neighbors = adj[u];
            for (auto it = neighbors.rbegin(); it != neighbors.rend(); ++it) {
                int v = *it;
                // --- Security: Check neighbor validity ---
                if (v >= 0 && v < V && !visited[v]) {
                    stack.push(v);
                }
            }
        }
    }
    return result;
}

// Helper to print a vector
void printVector(const std::vector<int>& vec) {
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << (i == vec.size() - 1 ? "" : ", ");
    }
    std::cout << "]" << std::endl;
}

int main() {
    std::cout << "--- 5 Test Cases for DFS ---" << std::endl;

    // Test Case 1: Connected Graph, Start 0
    int V1 = 7;
    std::vector<std::vector<int>> adj1(V1);
    GraphUtils::addEdge(adj1, 0, 1); GraphUtils::addEdge(adj1, 0, 3);
    GraphUtils::addEdge(adj1, 1, 2); GraphUtils::addEdge(adj1, 1, 4);
    GraphUtils::addEdge(adj1, 2, 1);
    GraphUtils::addEdge(adj1, 3, 0); GraphUtils::addEdge(adj1, 3, 4); GraphUtils::addEdge(adj1, 3, 5);
    GraphUtils::addEdge(adj1, 4, 1); GraphUtils::addEdge(adj1, 4, 3);
    GraphUtils::addEdge(adj1, 5, 3); GraphUtils::addEdge(adj1, 5, 6);
    GraphUtils::addEdge(adj1, 6, 5);
    std::cout << "Test Case 1 (Connected Graph, Start 0): ";
    printVector(dfs(V1, adj1, 0));

    // Test Case 2: Connected Graph, Start 4
    std::cout << "Test Case 2 (Connected Graph, Start 4): ";
    printVector(dfs(V1, adj1, 4));

    // Test Case 3: Disconnected Graph, Start 0
    int V3 = 5;
    std::vector<std::vector<int>> adj3(V3);
    GraphUtils::addEdge(adj3, 0, 1); GraphUtils::addEdge(adj3, 1, 0); GraphUtils::addEdge(adj3, 1, 2); GraphUtils::addEdge(adj3, 2, 1);
    GraphUtils::addEdge(adj3, 3, 4); GraphUtils::addEdge(adj3, 4, 3);
    std::cout << "Test Case 3 (Disconnected Graph, Start 0): ";
    printVector(dfs(V3, adj3, 0));

    // Test Case 4: Line Graph, Start 2
    int V4 = 5;
    std::vector<std::vector<int>> adj4(V4);
    GraphUtils::addEdge(adj4, 0, 1);
    GraphUtils::addEdge(adj4, 1, 0); GraphUtils::addEdge(adj4, 1, 2);
    GraphUtils::addEdge(adj4, 2, 1); GraphUtils::addEdge(adj4, 2, 3);
    GraphUtils::addEdge(adj4, 3, 2); GraphUtils::addEdge(adj4, 3, 4);
    GraphUtils::addEdge(adj4, 4, 3);
    std::cout << "Test Case 4 (Line Graph, Start 2): ";
    printVector(dfs(V4, adj4, 2));

    // Test Case 5: Single Node Graph, Start 0
    int V5 = 1;
    std::vector<std::vector<int>> adj5(V5);
    std::cout << "Test Case 5 (Single Node Graph, Start 0): ";
    printVector(dfs(V5, adj5, 0));

    return 0;
}