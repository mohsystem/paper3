#include <iostream>
#include <vector>
#include <list>

class Graph {
private:
    int V; // Number of vertices
    std::vector<std::list<int>> adj; // Adjacency list

    /**
     * A recursive helper function for DFS.
     * @param v The current vertex.
     * @param visited A boolean vector to track visited vertices.
     * @param result The vector to store the DFS traversal order.
     */
    void DFSUtil(int v, std::vector<bool>& visited, std::vector<int>& result) {
        visited[v] = true;
        result.push_back(v);

        for (int neighbor : adj[v]) {
            if (!visited[neighbor]) {
                DFSUtil(neighbor, visited, result);
            }
        }
    }

public:
    /**
     * Graph constructor.
     * @param V Number of vertices in the graph.
     */
    Graph(int V) {
        this->V = V;
        adj.resize(V);
    }

    /**
     * Adds an edge to the graph.
     * @param v The source vertex.
     * @param w The destination vertex.
     */
    void addEdge(int v, int w) {
        if (v < V && w < V) {
            adj[v].push_back(w);
        }
    }

    /**
     * Performs Depth First Search traversal starting from a given node.
     * @param startNode The starting node for the DFS.
     * @return A vector of integers representing the DFS traversal.
     */
    std::vector<int> performDFS(int startNode) {
        std::vector<int> result;
        if (startNode >= V) {
            std::cout << "Start node is out of bounds." << std::endl;
            return result; 
        }
        std::vector<bool> visited(V, false);
        DFSUtil(startNode, visited, result);
        return result;
    }
};

void printVector(const std::vector<int>& vec) {
    for (int val : vec) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
}

int main() {
    // Test Case 1: Simple linear graph
    std::cout << "Test Case 1:" << std::endl;
    Graph g1(4);
    g1.addEdge(0, 1);
    g1.addEdge(1, 2);
    g1.addEdge(2, 3);
    std::cout << "DFS starting from node 0: ";
    printVector(g1.performDFS(0));

    // Test Case 2: Graph with a fork
    std::cout << "\nTest Case 2:" << std::endl;
    Graph g2(5);
    g2.addEdge(0, 1);
    g2.addEdge(0, 2);
    g2.addEdge(1, 3);
    g2.addEdge(1, 4);
    std::cout << "DFS starting from node 0: ";
    printVector(g2.performDFS(0));

    // Test Case 3: Graph with a cycle
    std::cout << "\nTest Case 3:" << std::endl;
    Graph g3(4);
    g3.addEdge(0, 1);
    g3.addEdge(1, 2);
    g3.addEdge(2, 0);
    g3.addEdge(2, 3);
    std::cout << "DFS starting from node 0: ";
    printVector(g3.performDFS(0));

    // Test Case 4: Disconnected graph (DFS only visits connected component)
    std::cout << "\nTest Case 4:" << std::endl;
    Graph g4(5);
    g4.addEdge(0, 1);
    g4.addEdge(0, 2);
    g4.addEdge(3, 4);
    std::cout << "DFS starting from node 0: ";
    printVector(g4.performDFS(0));
    std::cout << "DFS starting from node 3: ";
    printVector(g4.performDFS(3));

    // Test Case 5: Single-node graph
    std::cout << "\nTest Case 5:" << std::endl;
    Graph g5(1);
    std::cout << "DFS starting from node 0: ";
    printVector(g5.performDFS(0));

    return 0;
}