#include <iostream>
#include <vector>
#include <queue>
#include <list>

/**
 * @brief Performs a Breadth-First Search on a graph.
 *
 * @param V The number of vertices in the graph.
 * @param adj The adjacency list representation of the graph.
 * @param startNode The starting node for the BFS.
 * @return A vector of integers representing the BFS traversal order.
 */
std::vector<int> bfs(int V, const std::vector<std::list<int>>& adj, int startNode) {
    std::vector<int> bfsTraversal;

    // Input validation
    if (V <= 0 || startNode < 0 || startNode >= V) {
        std::cerr << "Error: Invalid input. V is non-positive or startNode is out of bounds." << std::endl;
        return bfsTraversal; // Return empty vector for invalid input
    }

    std::vector<bool> visited(V, false);
    std::queue<int> queue;

    visited[startNode] = true;
    queue.push(startNode);

    while (!queue.empty()) {
        int u = queue.front();
        queue.pop();
        bfsTraversal.push_back(u);

        // Iterate over all adjacent vertices of u
        for (int v : adj[u]) {
            // Check bounds to be safe, although well-formed adj lists won't need this
            if (v >= 0 && v < V && !visited[v]) {
                visited[v] = true;
                queue.push(v);
            }
        }
    }

    return bfsTraversal;
}

// Helper function to add an edge to an undirected graph
void addEdge(std::vector<std::list<int>>& adj, int u, int v) {
    if (u < 0 || v < 0 || u >= adj.size() || v >= adj.size()) return;
    adj[u].push_back(v);
    adj[v].push_back(u);
}

void printTraversal(const std::vector<int>& traversal) {
    std::cout << " -> Traversal: [";
    for (size_t i = 0; i < traversal.size(); ++i) {
        std::cout << traversal[i] << (i == traversal.size() - 1 ? "" : ", ");
    }
    std::cout << "]" << std::endl;
}

int main() {
    const int V = 7;
    std::vector<std::list<int>> adj(V);

    // Creating a sample graph
    //      0 --- 1
    //      | \   |
    //      |  \  |
    //      2---3 4 --- 6
    //      |
    //      5
    addEdge(adj, 0, 1);
    addEdge(adj, 0, 2);
    addEdge(adj, 0, 3);
    addEdge(adj, 1, 4);
    addEdge(adj, 2, 3);
    addEdge(adj, 2, 5);
    addEdge(adj, 4, 6);

    // --- Test Cases ---
    int testStarts[] = {0, 3, 5, 6, 99}; // 99 is an invalid node

    for (int i = 0; i < 5; ++i) {
        int startNode = testStarts[i];
        std::cout << "Test Case " << (i + 1) << ": BFS starting from node " << startNode << std::endl;
        std::vector<int> result = bfs(V, adj, startNode);
        if (result.empty() && (startNode < 0 || startNode >= V)) {
            std::cout << " -> Handled invalid start node correctly." << std::endl;
        } else {
            printTraversal(result);
        }
        std::cout << "--------------------" << std::endl;
    }

    return 0;
}