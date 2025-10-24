#include <iostream>
#include <vector>
#include <list>
#include <algorithm> // For std::find

class Graph {
private:
    int V; // Number of vertices
    std::vector<std::list<int>> adj; // Adjacency list

    // A recursive helper function for DFS
    void dfsUtil(int v, std::vector<bool>& visited, std::vector<int>& result) {
        visited[v] = true;
        result.push_back(v);

        // Recur for all the vertices adjacent to this vertex
        for (int neighbor : adj[v]) {
            if (!visited[neighbor]) {
                dfsUtil(neighbor, visited, result);
            }
        }
    }

public:
    Graph(int V) {
        if (V < 0) {
            throw std::invalid_argument("Number of vertices must be non-negative");
        }
        this->V = V;
        adj.resize(V);
    }

    // Function to add an edge to the graph (undirected)
    void addEdge(int v, int w) {
        if (v < 0 || v >= V || w < 0 || w >= V) {
            std::cerr << "Error: Vertex out of bounds." << std::endl;
            return;
        }
        adj[v].push_back(w);
        adj[w].push_back(v); // For an undirected graph
    }

    // The main function that performs DFS traversal
    std::vector<int> dfs(int startNode) {
        std::vector<int> result;
        if (startNode < 0 || startNode >= V) {
            std::cerr << "Error: Start node is out of bounds." << std::endl;
            return result; // Return empty vector for invalid start node
        }
        
        std::vector<bool> visited(V, false);
        dfsUtil(startNode, visited, result);
        return result;
    }
};

// Helper function to print a vector
void printVector(const std::vector<int>& vec) {
    for (int val : vec) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
}

int main() {
    int numVertices = 8;
    Graph g(numVertices);

    // Create a sample graph
    g.addEdge(0, 1);
    g.addEdge(0, 2);
    g.addEdge(1, 3);
    g.addEdge(1, 4);
    g.addEdge(2, 5);
    g.addEdge(2, 6);
    g.addEdge(3, 7);
    // Node 7 is connected to 3

    std::cout << "Depth First Traversal (starting from various nodes):" << std::endl;

    // Test Case 1
    int startNode1 = 0;
    std::cout << "Test Case 1 (Start from " << startNode1 << "): ";
    std::vector<int> result1 = g.dfs(startNode1);
    printVector(result1);

    // Test Case 2
    int startNode2 = 3;
    std::cout << "Test Case 2 (Start from " << startNode2 << "): ";
    std::vector<int> result2 = g.dfs(startNode2);
    printVector(result2);

    // Test Case 3
    int startNode3 = 2;
    std::cout << "Test Case 3 (Start from " << startNode3 << "): ";
    std::vector<int> result3 = g.dfs(startNode3);
    printVector(result3);
    
    // Test Case 4
    int startNode4 = 6;
    std::cout << "Test Case 4 (Start from " << startNode4 << "): ";
    std::vector<int> result4 = g.dfs(startNode4);
    printVector(result4);

    // Test Case 5
    int startNode5 = 5;
    std::cout << "Test Case 5 (Start from " << startNode5 << "): ";
    std::vector<int> result5 = g.dfs(startNode5);
    printVector(result5);

    return 0;
}