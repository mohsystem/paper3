#include <iostream>
#include <vector>
#include <queue>

/**
 * Performs a Breadth-First Search on a graph.
 *
 * @param startNode The node to start the search from.
 * @param V         The total number of vertices in the graph.
 * @param adj       The adjacency list representation of the graph.
 * @return A vector of integers representing the BFS traversal order.
 */
std::vector<int> bfs(int startNode, int V, const std::vector<std::vector<int>>& adj) {
    std::vector<int> bfsTraversal;
    if (startNode >= V || V == 0) {
        return bfsTraversal; // Return empty if start node is invalid or graph is empty
    }
    std::vector<bool> visited(V, false);
    std::queue<int> q;

    visited[startNode] = true;
    q.push(startNode);

    while (!q.empty()) {
        int currentNode = q.front();
        q.pop();
        bfsTraversal.push_back(currentNode);

        for (int neighbor : adj[currentNode]) {
            if (!visited[neighbor]) {
                visited[neighbor] = true;
                q.push(neighbor);
            }
        }
    }
    return bfsTraversal;
}

// Helper function to add an edge to an undirected graph
void addEdge(std::vector<std::vector<int>>& adj, int u, int v) {
    adj[u].push_back(v);
    adj[v].push_back(u);
}

// Helper function to print a vector
void printVector(const std::vector<int>& vec) {
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << (i == vec.size() - 1 ? "" : ", ");
    }
    std::cout << "]" << std::endl;
}

int main() {
    // Test Case 1: Connected graph
    std::cout << "--- Test Case 1 ---" << std::endl;
    int V1 = 5;
    std::vector<std::vector<int>> adj1(V1);
    addEdge(adj1, 0, 1);
    addEdge(adj1, 0, 2);
    addEdge(adj1, 1, 3);
    addEdge(adj1, 2, 4);
    std::cout << "Graph: Connected graph with 5 vertices." << std::endl;
    std::vector<int> result1 = bfs(0, V1, adj1);
    std::cout << "BFS starting from node 0: ";
    printVector(result1);

    // Test Case 2: Different start node on a more complex graph
    std::cout << "\n--- Test Case 2 ---" << std::endl;
    int V2 = 5;
    std::vector<std::vector<int>> adj2(V2);
    addEdge(adj2, 0, 1);
    addEdge(adj2, 0, 2);
    addEdge(adj2, 1, 2);
    addEdge(adj2, 1, 3);
    addEdge(adj2, 2, 4);
    std::cout << "Graph: More complex connected graph." << std::endl;
    std::vector<int> result2 = bfs(3, V2, adj2);
    std::cout << "BFS starting from node 3: ";
    printVector(result2);

    // Test Case 3: Disconnected graph
    std::cout << "\n--- Test Case 3 ---" << std::endl;
    int V3 = 7;
    std::vector<std::vector<int>> adj3(V3);
    addEdge(adj3, 0, 1); addEdge(adj3, 0, 2); // Component 1
    addEdge(adj3, 3, 4); // Component 2
    addEdge(adj3, 5, 6); // Component 3
    std::cout << "Graph: Disconnected graph with 7 vertices." << std::endl;
    std::vector<int> result3 = bfs(3, V3, adj3);
    std::cout << "BFS starting from node 3: ";
    printVector(result3);

    // Test Case 4: Single node graph
    std::cout << "\n--- Test Case 4 ---" << std::endl;
    int V4 = 1;
    std::vector<std::vector<int>> adj4(V4);
    std::cout << "Graph: Single node graph." << std::endl;
    std::vector<int> result4 = bfs(0, V4, adj4);
    std::cout << "BFS starting from node 0: ";
    printVector(result4);

    // Test Case 5: Linear graph
    std::cout << "\n--- Test Case 5 ---" << std::endl;
    int V5 = 5;
    std::vector<std::vector<int>> adj5(V5);
    addEdge(adj5, 0, 1);
    addEdge(adj5, 1, 2);
    addEdge(adj5, 2, 3);
    addEdge(adj5, 3, 4);
    std::cout << "Graph: Linear graph with 5 vertices." << std::endl;
    std::vector<int> result5 = bfs(2, V5, adj5);
    std::cout << "BFS starting from node 2: ";
    printVector(result5);

    return 0;
}